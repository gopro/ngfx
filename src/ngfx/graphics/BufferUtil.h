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

namespace ngfx {
    static Buffer* createVertexBuffer(GraphicsContext* ctx, const void* data, uint32_t size) {
        return Buffer::create(ctx, data, size, BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    template <typename T> static inline Buffer* createVertexBuffer(GraphicsContext* ctx, const std::vector<T> &v) {
        return createVertexBuffer(ctx, v.data(), uint32_t(v.size() * sizeof(v[0])));
    }
    static Buffer* createIndexBuffer(GraphicsContext* ctx, const void* data, uint32_t size, uint32_t stride  = sizeof(uint32_t)) {
        return Buffer::create(ctx, data, size, BUFFER_USAGE_INDEX_BUFFER_BIT);
    }
    template <typename T> static inline Buffer* createIndexBuffer(GraphicsContext* ctx, const std::vector<T>& v, uint32_t stride = sizeof(uint32_t)) {
        return createIndexBuffer(ctx, v.data(), uint32_t(v.size() * sizeof(v[0])));
    }
    static Buffer* createUniformBuffer(GraphicsContext* ctx, const void* data, uint32_t size) {
        return Buffer::create(ctx, data, size, BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    }
    static Buffer* createStorageBuffer(GraphicsContext* ctx, const void* data, uint32_t size) {
        return Buffer::create(ctx, data, size, BUFFER_USAGE_STORAGE_BUFFER_BIT);
    }
};
