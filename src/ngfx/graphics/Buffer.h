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

    class Buffer {
    public:
        static Buffer* create(GraphicsContext* ctx, const void* data, uint32_t size, BufferUsageFlags usageFlags);
        template <typename T> static inline Buffer* create(GraphicsContext* ctx, const std::vector<T> &v,
                BufferUsageFlags usageFlags) {
            return create(ctx, v.data(), uint32_t(v.size() * sizeof(v[0])));
        }
        virtual ~Buffer() {}
        virtual void* map() = 0;
        virtual void unmap() = 0;
        virtual void upload(const void* data, uint32_t size, uint32_t offset = 0) = 0;
        virtual void download(void* data, uint32_t size, uint32_t offset = 0) = 0;
    };
};
