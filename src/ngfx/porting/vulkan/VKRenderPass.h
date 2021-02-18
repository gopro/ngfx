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
#include "ngfx/graphics/RenderPass.h"
#include "ngfx/porting/vulkan/VKFramebuffer.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>

namespace ngfx {
    class VKRenderPass : public RenderPass {
    public:
        void create(VkDevice device, const std::vector<VkAttachmentDescription> &attachmentsDesc,
                    const std::vector<VkSubpassDescription> &subpassesDesc,
                    const std::vector<VkSubpassDependency>& dependencies);
        virtual ~VKRenderPass();
        VkRenderPass v = VK_NULL_HANDLE;
        VkRenderPassCreateInfo createInfo;
        std::vector<VkAttachmentDescription> attachmentsDesc;
        std::vector<VkSubpassDescription> subpassesDesc;
        std::vector<VkSubpassDependency> dependencies;
    private:
        VkDevice device;
    };
    VK_CAST(RenderPass);
};
