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
#include "ngfx/core/DebugUtil.h"
#include "ngfx/graphics/Buffer.h"
#include "ngfx/porting/d3d/D3DDevice.h"

namespace ngfx {
class D3DGraphicsContext;
class D3DReadbackBuffer;

class D3DBuffer : public Buffer {
public:
  void create(D3DGraphicsContext *ctx, const void *data, uint32_t size,
              BufferUsageFlags bufferUsageFlags);
  void create(D3DGraphicsContext *ctx, const void *data, uint32_t size,
              D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
              D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE,
              D3D12_RESOURCE_STATES initialResourceState =
                  D3D12_RESOURCE_STATE_GENERIC_READ);
  virtual ~D3DBuffer();
  void *map() override;
  void unmap() override;
  void upload(const void *data, uint32_t size, uint32_t offset = 0) override;
  void download(void *data, uint32_t size, uint32_t offset = 0) override {
    NGFX_TODO("");
  }
  void setName(const std::string& name) override;
  ComPtr<ID3D12Resource> v;

protected:
  D3DGraphicsContext *ctx;
  D3D12_HEAP_TYPE heapType;
  D3D12_RESOURCE_STATES initialResourceState, currentResourceState;
  D3DReadbackBuffer *d3dReadbackBuffer = nullptr;
  void *d3dReadBackBufferPtr = nullptr;
};
D3D_CAST(Buffer);
} // namespace ngfx
