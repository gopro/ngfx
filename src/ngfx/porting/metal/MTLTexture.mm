/*
* Copyright 2020 GoPro Inc.
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*/

#include "ngfx/porting/metal/MTLTexture.h"
#include "ngfx/porting/metal/MTLGraphicsContext.h"
#include "ngfx/porting/metal/MTLCommandBuffer.h"
#include "ngfx/porting/metal/MTLRenderCommandEncoder.h"
#include "ngfx/core/DebugUtil.h"
using namespace ngfx;

void MTLTexture::create(MTLGraphicsContext *ctx, void* data, ::MTLPixelFormat format, uint32_t size,
        uint32_t w, uint32_t h, uint32_t d, uint32_t arrayLayers,
        MTLTextureUsage textureUsage, ::MTLTextureType textureType,
        bool genMipmaps, MTLSamplerDescriptor* samplerDescriptor, uint32_t numSamples) {
    this->ctx = ctx;
    this->w = w; this->h = h; this->d = d; this->arrayLayers = arrayLayers;
    this->textureType = ngfx::TextureType(textureType);
    this->format = PixelFormat(format);
    this->numSamples = numSamples;
    auto device = ctx->mtlDevice.v;
    MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor new];
    textureDescriptor.pixelFormat = format;
    textureDescriptor.width = w;
    textureDescriptor.height = h;
    textureDescriptor.depth = d;
    textureDescriptor.sampleCount = numSamples;
    
    if (textureType == ::MTLTextureTypeCube)
        textureDescriptor.arrayLength = 1;
    else textureDescriptor.arrayLength = arrayLayers;
    textureDescriptor.usage = textureUsage;
    if (numSamples > 1 && textureType == ::MTLTextureType2D)
        textureDescriptor.textureType = ::MTLTextureType2DMultisample;
    else if (numSamples > 1 && textureType == ::MTLTextureType2DArray)
    textureDescriptor.textureType = ::MTLTextureType2DMultisampleArray;
    else textureDescriptor.textureType = textureType;
    mipLevels = genMipmaps ? floor(log2(float(glm::min(w, h)))) + 1 : 1;
    textureDescriptor.mipmapLevelCount =  mipLevels;
    
    const std::vector<MTLPixelFormat> depthFormats = {
        MTLPixelFormatDepth16Unorm, MTLPixelFormatDepth24Unorm_Stencil8,
        MTLPixelFormatDepth32Float, MTLPixelFormatDepth32Float_Stencil8
    };
    const std::vector<MTLPixelFormat> stencilFormats = {
        MTLPixelFormatStencil8 ,MTLPixelFormatDepth24Unorm_Stencil8,
        MTLPixelFormatDepth32Float_Stencil8,
    };
    depthTexture = std::find(depthFormats.begin(), depthFormats.end(), format) != depthFormats.end();
    if (depthTexture) textureDescriptor.storageMode = ::MTLStorageModePrivate;
    stencilTexture = std::find(stencilFormats.begin(), stencilFormats.end(), format) != stencilFormats.end();
    bool multisampleTexture = (numSamples > 1);
    if (multisampleTexture) textureDescriptor.storageMode = ::MTLStorageModePrivate;
    v = [device newTextureWithDescriptor:textureDescriptor];
    
    upload(data, size);
    mtlSamplerState = [device newSamplerStateWithDescriptor:samplerDescriptor];
}

void MTLTexture::upload(void* data, uint32_t size, uint32_t x, uint32_t y, uint32_t z,
        int32_t w, int32_t h, int32_t d, int32_t arrayLayers) {
    if (!data) return;
    if (w == -1) w = this->w;
    if (h == -1) h = this->h;
    if (d == -1) d = this->d;
    if (arrayLayers == -1) arrayLayers = this->arrayLayers;
    NSUInteger bytesPerRow = size / (h * d * arrayLayers);
    NSUInteger bytesPerImage;
    if (MTLTextureType(textureType) == MTLTextureType3D) bytesPerImage = size / d;
    else if (MTLTextureType(textureType) == MTLTextureTypeCube) bytesPerImage = size / arrayLayers;
    else bytesPerImage = 0;
    MTLRegion region = MTLRegionMake3D(x, y, z, w, h, d);
    uint8_t* srcData = (uint8_t*)data;
    for (uint32_t slice = 0; slice < arrayLayers; slice++) {
        [v replaceRegion:region
            mipmapLevel:0
              slice:slice
              withBytes: srcData
            bytesPerRow:bytesPerRow
            bytesPerImage:bytesPerImage];
        srcData += bytesPerImage;
    }
    if (mipLevels != 1) {
        auto mtlCommandBuffer = [ctx->mtlCommandQueue commandBuffer];
        generateMipmapsFn(mtlCommandBuffer);
        [mtlCommandBuffer commit];
        [mtlCommandBuffer waitUntilCompleted];
    }
}

void MTLTexture::generateMipmaps(CommandBuffer* commandBuffer) {
    generateMipmapsFn(mtl(commandBuffer)->v);
}

void MTLTexture::generateMipmapsFn(id<MTLCommandBuffer> mtlCommandBuffer) {
    id <MTLBlitCommandEncoder> encoder = [mtlCommandBuffer blitCommandEncoder];
    [encoder generateMipmapsForTexture: v];
    [encoder endEncoding];
}

void MTLTexture::download(void* data, uint32_t size, uint32_t x, uint32_t y, uint32_t z,
          int32_t w, int32_t h, int32_t d, int32_t arrayLayers) {
    if (w == -1) w = this->w;
    if (h == -1) h = this->h;
    if (d == -1) d = this->d;
    
    id<MTLCommandBuffer> mtlCommandBuffer = [ctx->mtlCommandQueue commandBuffer];
    id <MTLBlitCommandEncoder> blitCommandEncoder = [mtlCommandBuffer blitCommandEncoder];
    [blitCommandEncoder synchronizeTexture:v slice:0 level:0];
    [blitCommandEncoder endEncoding];
    [mtlCommandBuffer commit];
    [mtlCommandBuffer waitUntilCompleted];
    
    NSUInteger bytesPerRow = 4 * w;
    MTLRegion region = { { x, y, z }, { NSUInteger(w), NSUInteger(h), 1 } };
    [v getBytes:data bytesPerRow: bytesPerRow fromRegion: region mipmapLevel: 0];
}

Texture* Texture::create(GraphicsContext* ctx, Graphics* graphics, void* data, PixelFormat format, uint32_t size,
         uint32_t w, uint32_t h, uint32_t d, uint32_t arrayLayers, ImageUsageFlags imageUsageFlags,
         TextureType textureType, bool genMipmaps, FilterMode minFilter, FilterMode magFilter, FilterMode mipFilter,
         uint32_t numSamples) {
    MTLTexture* mtlTexture = new MTLTexture();
    MTLTextureUsage textureUsage = 0;
    if (imageUsageFlags & IMAGE_USAGE_SAMPLED_BIT) textureUsage |= MTLTextureUsageShaderRead;
    if (imageUsageFlags & IMAGE_USAGE_COLOR_ATTACHMENT_BIT) textureUsage |= MTLTextureUsageRenderTarget;
    else if (imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) textureUsage |= MTLTextureUsageRenderTarget;
    MTLSamplerDescriptor *mtlSamplerDescriptor = [MTLSamplerDescriptor new];
    mtlSamplerDescriptor.minFilter = ::MTLSamplerMinMagFilter(minFilter);
    mtlSamplerDescriptor.magFilter = ::MTLSamplerMinMagFilter(magFilter);
    mtlSamplerDescriptor.mipFilter = (mipFilter == FILTER_NEAREST) ? MTLSamplerMipFilterNearest : MTLSamplerMipFilterLinear;
    mtlTexture->create(mtl(ctx), data, ::MTLPixelFormat(format), size, w, h, d, arrayLayers,
       textureUsage, ::MTLTextureType(textureType), genMipmaps, mtlSamplerDescriptor, numSamples);
    return mtlTexture;
}
