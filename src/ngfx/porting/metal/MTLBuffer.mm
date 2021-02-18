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

#include "ngfx/porting/metal/MTLBuffer.h"
#include "ngfx/porting/metal/MTLGraphicsContext.h"
#include "ngfx/core/DebugUtil.h"
using namespace ngfx;

void MTLBuffer::create(MTLGraphicsContext* ctx, const void* data, uint32_t size, MTLResourceOptions resourceOptions) {
    if (!data) v = [ctx->mtlDevice.v newBufferWithLength:size options:resourceOptions];
    else v = [ctx->mtlDevice.v newBufferWithBytes:data length:size options:resourceOptions];
}

Buffer* Buffer::create(GraphicsContext* ctx, const void* data, uint32_t size, BufferUsageFlags usageFlags) {
    MTLBuffer* buffer = new MTLBuffer();
    buffer->create(mtl(ctx), data, size, MTLResourceStorageModeShared);
    return buffer;
}

void* MTLBuffer::map() { return v.contents; }
void MTLBuffer::unmap() {}
void MTLBuffer::upload(const void* data, uint32_t size, uint32_t offset) {
    memcpy((uint8_t*)(v.contents) + offset, data, size);
}
void MTLBuffer::download(void* data, uint32_t size, uint32_t offset) {
    memcpy(data, (uint8_t*)(v.contents) + offset, size);
}
