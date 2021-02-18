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
#include "ngfx/graphics/Fence.h"
#include "ngfx/porting/d3d/D3DUtil.h"

namespace ngfx {
    class D3DFence : public Fence {
    public:
        enum Value { UNSIGNALED, SIGNALED };
        void create(ID3D12Device* device, Value initialValue = UNSIGNALED);
        virtual ~D3DFence() {}
        void wait() override;
        void reset() override;
        ComPtr<ID3D12Fence> v;
        HANDLE fenceEvent;
    private:
        ID3D12Device* device;
    };
    D3D_CAST(Fence);
};
