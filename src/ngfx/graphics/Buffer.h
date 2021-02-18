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
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/GraphicsCore.h"
#include <cstdint>
#include <vector>

namespace ngfx {
class GraphicsContext;

/** \class Buffer
 * 
 *  This class defines the interface for a graphics buffer.
 *  Each backend is responsible for implementing this interface via a subclass.
 *  The same buffer object can support multiple usage scenarios,
 *  including storing uniform data, vertex buffer data, index buffer data, etc.
 *  In addition, on shared memory architectures, it supports shared CPU/GPU access. 
 */

class Buffer {
public:
  /** Create a graphics buffer
   *  @param ctx The graphics context
   *  @param data The buffer data
   *  @param size The size of input data (in bytes)
   *  @param usageFlags The buffer usage flags
   */
  static Buffer *create(GraphicsContext *ctx, const void *data, uint32_t size,
                        BufferUsageFlags usageFlags);
  template <typename T>
  static inline Buffer *create(GraphicsContext *ctx, const std::vector<T> &v,
                               BufferUsageFlags usageFlags) {
    return create(ctx, v.data(), uint32_t(v.size() * sizeof(v[0])));
  }
  /** Destroy the buffer */
  virtual ~Buffer() {}
  /** Map the buffer contents for CPU read/write access */
  virtual void *map() = 0;
  /** Unmap the buffer */
  virtual void unmap() = 0;
  /** Upload the CPU data to the GPU buffer
   *  @param data The buffer data
   *  @param size The size of the data (in bytes)
   *  @param offset The destination offset (in bytes) */ 
  virtual void upload(const void *data, uint32_t size, uint32_t offset = 0) = 0;
  /** Download the GPU data to CPU-accessible memory
   *  @param data The destination address
   *  @param size The size of the data to download (in bytes)
   *  @param offset The destination offset (in bytes) */
  virtual void download(void *data, uint32_t size, uint32_t offset = 0) = 0;
};
}; // namespace ngfx
