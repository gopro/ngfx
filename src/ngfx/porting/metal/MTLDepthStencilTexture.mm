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

#include "ngfx/porting/metal/MTLDepthStencilTexture.h"
#include "ngfx/porting/metal/MTLGraphicsContext.h"
using namespace ngfx;

void MTLDepthStencilTexture::create(MTLGraphicsContext* ctx, uint32_t w, uint32_t h, ::MTLPixelFormat fmt) {
    this->format = fmt;
    auto device = ctx->mtlDevice.v;
    MTLTextureDescriptor *desc = [MTLTextureDescriptor new];
    desc.width = w;
    desc.height = h;
    desc.mipmapLevelCount = 1;
    desc.storageMode = MTLStorageModePrivate;
    desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
    desc.pixelFormat = fmt;
    v = [device newTextureWithDescriptor:desc];
}

