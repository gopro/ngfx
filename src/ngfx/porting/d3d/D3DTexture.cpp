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
#include "ngfx/porting/d3d/D3DTexture.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/porting/d3d/D3DBlitOp.h"
#include "ngfx/porting/d3d/D3DBuffer.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
using namespace ngfx;
using namespace std;

void D3DTexture::create(D3DGraphicsContext *ctx, D3DGraphics *graphics,
                        void *data, uint32_t size, uint32_t w, uint32_t h,
                        uint32_t d, uint32_t arrayLayers, DXGI_FORMAT format,
                        ImageUsageFlags usageFlags, TextureType textureType,
                        bool genMipmaps, uint32_t numSamples,
                        const D3DSamplerDesc &samplerDesc) {
  this->ctx = ctx;
  this->graphics = graphics;
  this->w = w;
  this->h = h;
  this->d = d;
  this->arrayLayers = arrayLayers;
  this->size = size;
  this->format = PixelFormat(format);
  this->textureType = textureType;
  this->mipLevels =
      genMipmaps ? uint32_t(floor(log2(float(glm::min(w, h))))) + 1 : 1;
  if (genMipmaps)
    usageFlags |= IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  this->imageUsageFlags = usageFlags;
  this->numSamples = numSamples;
  numSubresources = arrayLayers * mipLevels;
  currentResourceState.resize(numSubresources);

  HRESULT hResult;
  D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
  if (imageUsageFlags & IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  if (imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  auto d3dDevice = ctx->d3dDevice.v.Get();
  if (textureType == TEXTURE_TYPE_3D) {
    resourceDesc =
        CD3DX12_RESOURCE_DESC::Tex3D(format, w, h, d, 1, resourceFlags);
  } else {
    auto texFormat = format;
    if (texFormat == DXGI_FORMAT_D16_UNORM &&
        (usageFlags & IMAGE_USAGE_SAMPLED_BIT))
      texFormat = DXGI_FORMAT_R16_TYPELESS;
    else if (texFormat == DXGI_FORMAT_D24_UNORM_S8_UINT &&
             (usageFlags & IMAGE_USAGE_SAMPLED_BIT))
      texFormat = DXGI_FORMAT_R24G8_TYPELESS;
    resourceDesc =
        CD3DX12_RESOURCE_DESC::Tex2D(texFormat, w, h, d * arrayLayers,
                                     mipLevels, numSamples, 0, resourceFlags);
  }
  bool isRenderTarget =
      (resourceFlags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
  D3D12_CLEAR_VALUE clearValue = {format, {0.0f, 0.0f, 0.0f, 0.0f}};
  CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
  V(d3dDevice->CreateCommittedResource(
      &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
      D3D12_RESOURCE_STATE_COPY_DEST, isRenderTarget ? &clearValue : nullptr,
      IID_PPV_ARGS(&v)));

  for (auto &s : currentResourceState)
    s = D3D12_RESOURCE_STATE_COPY_DEST;

  if (imageUsageFlags & IMAGE_USAGE_SAMPLED_BIT) {
    defaultSrvDescriptor = getSrvDescriptor(0, mipLevels);
    defaultSamplerDescriptor = getSamplerDescriptor(samplerDesc.Filter);
  }

  if (isRenderTarget) {
    defaultRtvDescriptor = getRtvDescriptor();
  }

  if (imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
    // Create depth stencil view
    auto dsvDescriptorHeap = &ctx->d3dDsvDescriptorHeap;
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = format;
    dsvDesc.ViewDimension = (numSamples > 1) ? D3D12_DSV_DIMENSION_TEXTURE2DMS
                                             : D3D12_DSV_DIMENSION_TEXTURE2D;
    D3D_TRACE(d3dDevice->CreateDepthStencilView(
        v.Get(), &dsvDesc, dsvDescriptorHeap->handle.cpuHandle));
    dsvDescriptor = dsvDescriptorHeap->handle;
    ++dsvDescriptorHeap->handle;
  }

  upload(data, size);
}

D3DDescriptorHandle D3DTexture::getSamplerDescriptor(D3D12_FILTER filter) {
  for (auto &sampler : samplerCache) {
    if (sampler.desc.Filter == filter)
      return sampler.handle;
  }
  // Create sampler
  D3DSamplerDesc samplerDesc;
  samplerDesc.Filter = filter;
  D3DSampler sampler;
  sampler.create(ctx, samplerDesc);
  D3DDescriptorHandle result = sampler.handle;
  samplerCache.emplace_back(std::move(sampler));
  return result;
}

D3DDescriptorHandle D3DTexture::getSrvDescriptor(uint32_t baseMipLevel,
                                                 uint32_t numMipLevels) {
  for (auto &srvData : srvDescriptorCache) {
    if (textureType == TEXTURE_TYPE_2D &&
        srvData.desc.Texture2D.MostDetailedMip == baseMipLevel &&
        srvData.desc.Texture2D.MipLevels == numMipLevels)
      return srvData.handle;
    else if (textureType == TEXTURE_TYPE_2D_ARRAY &&
             srvData.desc.Texture2DArray.MostDetailedMip == baseMipLevel &&
             srvData.desc.Texture2DArray.MipLevels == numMipLevels)
      return srvData.handle;
    if (textureType == TEXTURE_TYPE_3D &&
        srvData.desc.Texture3D.MostDetailedMip == baseMipLevel &&
        srvData.desc.Texture3D.MipLevels == numMipLevels)
      return srvData.handle;
    else if (textureType == TEXTURE_TYPE_CUBE &&
             srvData.desc.TextureCube.MostDetailedMip == baseMipLevel &&
             srvData.desc.TextureCube.MipLevels == numMipLevels)
      return srvData.handle;
  }
  // Create a shader resource view
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format = resourceDesc.Format;
  if (srvDesc.Format == DXGI_FORMAT_R16_TYPELESS)
    srvDesc.Format = DXGI_FORMAT_R16_UNORM;
  else if (srvDesc.Format == DXGI_FORMAT_R24G8_TYPELESS)
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION(textureType);
  if (textureType == TEXTURE_TYPE_2D) {
    srvDesc.Texture2D.MostDetailedMip = baseMipLevel;
    srvDesc.Texture2D.MipLevels = numMipLevels;
  } else if (textureType == TEXTURE_TYPE_2D_ARRAY) {
    srvDesc.Texture2DArray.MostDetailedMip = baseMipLevel;
    srvDesc.Texture2DArray.MipLevels = numMipLevels;
  } else if (textureType == TEXTURE_TYPE_3D) {
    srvDesc.Texture3D.MostDetailedMip = baseMipLevel;
    srvDesc.Texture3D.MipLevels = numMipLevels;
  } else if (textureType == TEXTURE_TYPE_CUBE) {
    srvDesc.TextureCube.MostDetailedMip = baseMipLevel;
    srvDesc.TextureCube.MipLevels = numMipLevels;
  }
  auto &cbvSrvUavDescriptorHeap = ctx->d3dCbvSrvUavDescriptorHeap;
  auto d3dDevice = ctx->d3dDevice.v.Get();
  D3D_TRACE(d3dDevice->CreateShaderResourceView(
      v.Get(), &srvDesc, cbvSrvUavDescriptorHeap.handle.cpuHandle));
  SrvData srvData;
  srvData.desc = srvDesc;
  srvData.handle = cbvSrvUavDescriptorHeap.handle;
  ++cbvSrvUavDescriptorHeap.handle;
  auto result = srvData.handle;
  srvDescriptorCache.emplace_back(std::move(srvData));
  return result;
}

D3D12_RENDER_TARGET_VIEW_DESC
getRtvDesc(TextureType textureType, DXGI_FORMAT format, uint32_t numSamples,
           uint32_t level, uint32_t baseLayer, uint32_t layerCount) {
  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
  rtvDesc.Format = DXGI_FORMAT(format);
  if (textureType == TEXTURE_TYPE_2D) {
    if (numSamples > 1) {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
    } else {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
      rtvDesc.Texture2D.MipSlice = level;
    }
  } else if (textureType == TEXTURE_TYPE_2D_ARRAY ||
             textureType == TEXTURE_TYPE_CUBE) {
    if (numSamples > 1) {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
      rtvDesc.Texture2DMSArray.FirstArraySlice = baseLayer;
      rtvDesc.Texture2DMSArray.ArraySize = layerCount;
    } else {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      rtvDesc.Texture2DArray.MipSlice = level;
      rtvDesc.Texture2DArray.FirstArraySlice = baseLayer;
      rtvDesc.Texture2DArray.ArraySize = layerCount;
    }
  } else if (textureType == TEXTURE_TYPE_3D) {
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
    rtvDesc.Texture3D.MipSlice = level;
  }
  return rtvDesc;
}
D3DDescriptorHandle D3DTexture::getRtvDescriptor(uint32_t level,
                                                 uint32_t baseLayer,
                                                 uint32_t layerCount) {
  for (auto &rtvData : rtvDescriptorCache) {
    if (textureType == TEXTURE_TYPE_2D && numSamples > 1)
      return rtvData.handle;
    else if (textureType == TEXTURE_TYPE_2D &&
             rtvData.desc.Texture2D.MipSlice == level)
      return rtvData.handle;
    else if ((textureType == TEXTURE_TYPE_2D_ARRAY ||
              textureType == TEXTURE_TYPE_CUBE) &&
             numSamples > 1 &&
             rtvData.desc.Texture2DMSArray.FirstArraySlice == baseLayer &&
             rtvData.desc.Texture2DMSArray.ArraySize == layerCount)
      return rtvData.handle;
    else if ((textureType == TEXTURE_TYPE_2D_ARRAY ||
              textureType == TEXTURE_TYPE_CUBE) &&
             rtvData.desc.Texture2DArray.MipSlice == level &&
             rtvData.desc.Texture2DArray.FirstArraySlice == baseLayer &&
             rtvData.desc.Texture2DArray.ArraySize == layerCount)
      return rtvData.handle;
    else if (textureType == TEXTURE_TYPE_3D &&
             rtvData.desc.Texture3D.MipSlice == level)
      return rtvData.handle;
  }
  // Create render target view
  auto rtvDescriptorHeap = &ctx->d3dRtvDescriptorHeap;
  auto d3dDevice = ctx->d3dDevice.v.Get();
  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc =
      getRtvDesc(textureType, DXGI_FORMAT(format), numSamples, level, baseLayer,
                 layerCount);
  D3D_TRACE(d3dDevice->CreateRenderTargetView(
      v.Get(), &rtvDesc, rtvDescriptorHeap->handle.cpuHandle));
  RtvData rtvData;
  rtvData.desc = rtvDesc;
  rtvData.handle = rtvDescriptorHeap->handle;
  ++rtvDescriptorHeap->handle;
  auto result = rtvData.handle;
  rtvDescriptorCache.emplace_back(std::move(rtvData));
  return result;
}

void D3DTexture::generateMipmapsFn(D3DCommandList *cmdList) {
  genMipmapData.reset(new GenMipmapData());
  for (uint32_t j = 1; j < mipLevels; j++) {
    D3DBlitOp op(
        ctx, this, j - 1, this, j,
        {{0, 0, 0},
         {int32_t(glm::max(w >> (j - 1), 1u)),
          int32_t(glm::max(h >> (j - 1), 1u)), 1}},
        {{0, 0, 0},
         {int32_t(glm::max(w >> j, 1u)), int32_t(glm::max(h >> j, 1u)), 1}},
        0, arrayLayers, 0, arrayLayers);
    op.apply(ctx, cmdList, graphics);
    genMipmapData->ops.emplace_back(std::move(op));
  }
}

void D3DTexture::upload(void *data, uint32_t size, uint32_t x, uint32_t y,
                        uint32_t z, int32_t w, int32_t h, int32_t d,
                        int32_t arrayLayers) {
  auto &copyCommandList = ctx->d3dCopyCommandList;
  std::unique_ptr<D3DBuffer> stagingBuffer;
  if (w == -1)
    w = this->w;
  if (h == -1)
    h = this->h;
  if (d == -1)
    d = this->d;
  if (arrayLayers == -1)
    arrayLayers = this->arrayLayers;
  if (data) {
    uint64_t stagingBufferSize;
    D3D_TRACE(stagingBufferSize =
                  GetRequiredIntermediateSize(v.Get(), 0, arrayLayers));
    stagingBuffer.reset(new D3DBuffer());
    stagingBuffer->create(ctx, nullptr, uint32_t(stagingBufferSize),
                          D3D12_HEAP_TYPE_UPLOAD);
  }
  copyCommandList.begin();
  uploadFn(&copyCommandList, data, size, stagingBuffer.get(), x, y, z, w, h, d,
           arrayLayers);
  D3D12_RESOURCE_STATES resourceState =
      (imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
          ? D3D12_RESOURCE_STATE_DEPTH_WRITE
          : D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  resourceBarrier(&copyCommandList, resourceState);
  copyCommandList.end();
  ctx->d3dCommandQueue.submit(copyCommandList.v.Get(), nullptr);
  ctx->d3dCommandQueue.waitIdle();

  if (data && mipLevels != 1) {
    D3DCommandList cmdList;
    ComPtr<ID3D12CommandAllocator> cmdAllocator;
    HRESULT hResult;
    auto d3dDevice = ctx->d3dDevice.v.Get();
    cmdList.create(d3dDevice);
    cmdList.begin();
    generateMipmapsFn(&cmdList);
    cmdList.end();
    ctx->d3dCommandQueue.submit(cmdList.v.Get(), nullptr);
    ctx->d3dCommandQueue.waitIdle();
  }
}

void D3DTexture::generateMipmaps(CommandBuffer *commandBuffer) {
  generateMipmapsFn((D3DCommandList *)commandBuffer);
}

void D3DTexture::uploadFn(D3DCommandList *cmdList, void *data, uint32_t size,
                          D3DBuffer *stagingBuffer, uint32_t x, uint32_t y,
                          uint32_t z, int32_t w, int32_t h, int32_t d,
                          int32_t arrayLayers) {
  if (data) {
    if (x != 0 || y != 0 || z != 0)
      NGFX_LOG_TRACE("TODO: support sub-region update");
    resourceBarrier(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);
    uint32_t rowPitch = size / (h * d * arrayLayers);
    uint32_t slicePitch = size / (d * arrayLayers);
    vector<D3D12_SUBRESOURCE_DATA> textureData(arrayLayers);
    uint8_t *srcData = (uint8_t *)data;
    for (uint32_t j = 0; j < uint32_t(arrayLayers); j++) {
      textureData[j] = {srcData, long(rowPitch), slicePitch};
      srcData += slicePitch;
    }
    uint64_t bufferSize =
        UpdateSubresources(cmdList->v.Get(), v.Get(), stagingBuffer->v.Get(), 0,
                           0, arrayLayers, textureData.data());
    assert(bufferSize);
  }
}

void D3DTexture::download(void *data, uint32_t size, uint32_t x, uint32_t y,
                          uint32_t z, int32_t w, int32_t h, int32_t d,
                          int32_t arrayLayers) {
  auto &copyCommandList = ctx->d3dCopyCommandList;
  const bool flipY = true; //TODO:move to param
  if (w == -1)
    w = this->w;
  if (h == -1)
    h = this->h;
  if (d == -1)
    d = this->d;

  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
  uint32_t numRows;
  uint64_t srcSize, rowSizeBytes;
  D3D12_RESOURCE_DESC desc = v->GetDesc();
  D3D_TRACE(ctx->d3dDevice.v->GetCopyableFootprints(
      &desc, 0, 1, 0, &footprint, &numRows, &rowSizeBytes, &srcSize));

  D3DReadbackBuffer readbackBuffer;
  readbackBuffer.create(ctx, uint32_t(srcSize));

  D3D12_BOX srcRegion = {0, 0, 0, UINT(w), UINT(h), 1};

  copyCommandList.begin();
  downloadFn(&copyCommandList, readbackBuffer, srcRegion, footprint);
  copyCommandList.end();
  ctx->d3dCommandQueue.submit(&copyCommandList);
  ctx->d3dCommandQueue.waitIdle();

  void *readbackBufferPtr = readbackBuffer.map();
  uint8_t *srcPtr = (uint8_t *)readbackBufferPtr,
          *dstPtr =
              &((uint8_t *)data)[flipY ? (uint64_t(h) - 1) * rowSizeBytes : 0];
  int64_t dstInc = flipY ? -rowSizeBytes : rowSizeBytes;
  for (uint32_t j = 0; j < uint32_t(h); j++) {
    memcpy(dstPtr, srcPtr, rowSizeBytes);
    srcPtr += footprint.Footprint.RowPitch;
    dstPtr += dstInc;
  }
  readbackBuffer.unmap();
}

void D3DTexture::downloadFn(D3DCommandList *cmdList,
                            D3DReadbackBuffer &readbackBuffer,
                            D3D12_BOX &srcRegion,
                            D3D12_PLACED_SUBRESOURCE_FOOTPRINT &dstFootprint) {
  resourceBarrier(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);

  D3D12_TEXTURE_COPY_LOCATION dstLocation = {
      readbackBuffer.v.Get(),
      D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
      {dstFootprint}};
  D3D12_TEXTURE_COPY_LOCATION srcLocation = {
      v.Get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0};

  D3D_TRACE(cmdList->v->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation,
                                          &srcRegion));
  D3D12_RESOURCE_STATES resourceState =
      (imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
          ? D3D12_RESOURCE_STATE_DEPTH_WRITE
          : D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  resourceBarrier(cmdList, resourceState);
  for (auto &s : currentResourceState)
    s = resourceState;
}

void D3DTexture::changeLayout(CommandBuffer *commandBuffer,
                              ImageLayout imageLayout) {
  D3D12_RESOURCE_STATES resourceState;
  switch (imageLayout) {
  case IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    break;
  case IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    break;
  case IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    resourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    break;
  case IMAGE_LAYOUT_GENERAL:
    resourceState = D3D12_RESOURCE_STATE_COMMON;
    break;
  };

  resourceBarrier(d3d(commandBuffer), resourceState);
}
void D3DTexture::resourceBarrier(D3DCommandList *cmdList,
                                 D3D12_RESOURCE_STATES newState,
                                 UINT subresource) {
  uint32_t j0, j1;
  if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
    j0 = 0;
    j1 = numSubresources;
  } else {
    j0 = subresource;
    j1 = j0 + 1;
  }
  for (uint32_t j = j0; j < j1; j++) {
    if (currentResourceState[j] == newState)
      continue;
    CD3DX12_RESOURCE_BARRIER resourceBarrier =
        CD3DX12_RESOURCE_BARRIER::Transition(v.Get(), currentResourceState[j],
                                             newState, j);
    D3D_TRACE(cmdList->v->ResourceBarrier(1, &resourceBarrier));
    currentResourceState[j] = newState;
  }
}

Texture *Texture::create(GraphicsContext *ctx, Graphics *graphics, void *data,
                         PixelFormat format, uint32_t size, uint32_t w,
                         uint32_t h, uint32_t d, uint32_t arrayLayers,
                         ImageUsageFlags imageUsageFlags,
                         TextureType textureType, bool genMipmaps,
                         FilterMode minFilter, FilterMode magFilter,
                         FilterMode mipFilter, uint32_t numSamples) {
  D3DTexture *d3dTexture = new D3DTexture();
  D3DSamplerDesc samplerDesc;
  uint32_t filter = minFilter << 2 | magFilter << 1 | mipFilter;
  static D3D12_FILTER filterMap[] = {
      D3D12_FILTER_MIN_MAG_MIP_POINT,
      D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
      D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
      D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR,
      D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT,
      D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
      D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
      D3D12_FILTER_MIN_MAG_MIP_LINEAR};
  samplerDesc.Filter = filterMap[filter];
  d3dTexture->create(d3d(ctx), (D3DGraphics *)graphics, data, size, w, h, d,
                     arrayLayers, DXGI_FORMAT(format), imageUsageFlags,
                     textureType, genMipmaps, numSamples, samplerDesc);
  return d3dTexture;
}
