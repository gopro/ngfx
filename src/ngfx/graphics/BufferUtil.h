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
#include "ngfx/graphics/Buffer.h"

/** \class BufferUtil
 * 
 *  This utility module provides helper functions for creating various types of 
 *  specialized GPU buffers
 */

namespace ngfx {
  /** Create a vertex buffer
   *  @param ctx The graphics context
   *  @param data The buffer data
   *  @param size The size of input data (in bytes)
   */
  static Buffer *createVertexBuffer(GraphicsContext *ctx, const void *data,
                                    uint32_t size) {
    return Buffer::create(ctx, data, size, BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
  /** Create a vertex buffer
   *  @param ctx The graphics context
   *  @param v The buffer data
   */
  template <typename T>
  static inline Buffer *createVertexBuffer(GraphicsContext *ctx,
                                           const std::vector<T> &v) {
    return createVertexBuffer(ctx, v.data(), uint32_t(v.size() * sizeof(v[0])));
  }
  /** Create an index buffer
   *  @param ctx The graphics context
   *  @param data The buffer data
   *  @param size The buffer size (in bytes)
   *  @param stride The stride of the input data (in bytes)
   */
  static Buffer *createIndexBuffer(GraphicsContext *ctx, const void *data,
                                   uint32_t size,
                                   uint32_t stride = sizeof(uint32_t)) {
    return Buffer::create(ctx, data, size, BUFFER_USAGE_INDEX_BUFFER_BIT);
  }
  /** Create an index buffer
   *  @param ctx The graphics context
   *  @param v The buffer data
   *  @param stride The stride of the input data (in bytes)
   */
  template <typename T>
  static inline Buffer *createIndexBuffer(GraphicsContext *ctx,
                                          const std::vector<T> &v,
                                          uint32_t stride = sizeof(uint32_t)) {
    return createIndexBuffer(ctx, v.data(), uint32_t(v.size() * sizeof(v[0])));
  }
  /** Create a uniform buffer
   *  @param ctx The graphics context
   *  @param data The buffer data
   *  @param size The buffer size (in bytes)
   */
  static Buffer *createUniformBuffer(GraphicsContext *ctx, const void *data,
                                     uint32_t size) {
    return Buffer::create(ctx, data, size, BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  }
  /** Create a storage buffer
   *  @param ctx The graphics context
   *  @param data The buffer data
   *  @param size The buffer size (in bytes)
   */
  static Buffer *createStorageBuffer(GraphicsContext *ctx, const void *data,
                                     uint32_t size) {
    return Buffer::create(ctx, data, size, BUFFER_USAGE_STORAGE_BUFFER_BIT);
  }
}; // namespace ngfx
