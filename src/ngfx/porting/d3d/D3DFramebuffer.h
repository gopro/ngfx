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
#include "ngfx/graphics/Framebuffer.h"
#include "ngfx/porting/d3d/D3DUtil.h"

namespace ngfx {
    class D3DTexture;
    class D3DFramebuffer : public Framebuffer {
    public:
        struct D3DAttachment { 
            ID3D12Resource* resource; 
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor; 
            uint32_t subresourceIndex = 0;
            uint32_t imageUsageFlags = 0;
            uint32_t numSamples = 1;
            DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
            D3DTexture* texture = nullptr;
        };
        void create(std::vector<D3DAttachment>& attachments, int32_t w, uint32_t h, uint32_t layers = 1);
        virtual ~D3DFramebuffer() {}
        std::vector<D3DAttachment> d3dAttachments;
        std::vector<D3DAttachment*> colorAttachments, resolveAttachments;
        D3DAttachment* depthStencilAttachment = nullptr, *depthResolve = nullptr;
    };
    D3D_CAST(Framebuffer);
}