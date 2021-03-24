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
#pragma once
#include "ngfx/graphics/Texture.h"
#include "ngfx/porting/metal/MTLUtil.h"
#include <Metal/Metal.h>
#include "ngfx/core/DebugUtil.h"

namespace ngfx {
    class MTLGraphicsContext;
    class MTLTexture : public Texture {
    public:
        void create(MTLGraphicsContext *ctx, void* data, ::MTLPixelFormat format, uint32_t size,
            uint32_t w, uint32_t h, uint32_t d, uint32_t arrayLayers,
            MTLTextureUsage textureUsage, ::MTLTextureType textureType,
            bool genMipmaps, MTLSamplerDescriptor* samplerDescriptor, uint32_t numSamples);
        virtual ~MTLTexture();
        void upload(void* data, uint32_t size, uint32_t x = 0, uint32_t y = 0, uint32_t z = 0,
                    int32_t w = -1, int32_t h = -1, int32_t d = -1, int32_t arrayLayers = -1) override;
        void download(void* data, uint32_t size, uint32_t x = 0, uint32_t y = 0, uint32_t z = 0,
                      int32_t w = -1, int32_t h = -1, int32_t d = -1, int32_t arrayLayers = -1) override;
        void changeLayout(CommandBuffer* commandBuffer, ImageLayout imageLayout) override {}
        void generateMipmaps(CommandBuffer* commandBuffer) override;
        MTLGraphicsContext* ctx = nullptr;
        id<MTLTexture> v;
        id<MTLSamplerState> mtlSamplerState;
        bool depthTexture = false, stencilTexture = false;
    private:
        void generateMipmapsFn(id<MTLCommandBuffer> mtlCommandBuffer);
    };
    MTL_CAST(Texture);
}
