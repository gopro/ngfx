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
#include "ngfx/graphics/ShaderModule.h"
#include "ngfx/porting/metal/MTLDevice.h"
#include "ngfx/porting/metal/MTLUtil.h"
#include <Metal/Metal.h>

namespace ngfx {
    class MTLShaderModule {
    public:
        virtual void initFromFile(id<MTLDevice> device, const std::string& filename);
        virtual ~MTLShaderModule() {}
        id<MTLLibrary> mtlLibrary;
        id<MTLFunction> mtlFunction;
    protected:
        virtual void initFromByteCode(id<MTLDevice> device, void* data, uint32_t size);
    };
    MTL_CAST(ShaderModule);

    class MTLVertexShaderModule : public VertexShaderModule, public MTLShaderModule {
    public:
        virtual ~MTLVertexShaderModule() {}
    };
    MTL_CAST(VertexShaderModule);

    class MTLFragmentShaderModule : public FragmentShaderModule, public MTLShaderModule {
    public:
        virtual ~MTLFragmentShaderModule() {}
    };
    MTL_CAST(FragmentShaderModule);

    class MTLComputeShaderModule : public ComputeShaderModule, public MTLShaderModule {
    public:
        virtual ~MTLComputeShaderModule() {}
    };
    MTL_CAST(ComputeShaderModule);
}
