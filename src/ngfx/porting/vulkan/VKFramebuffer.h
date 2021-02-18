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
#include "ngfx/porting/vulkan/VKImageView.h"
#include "ngfx/porting/vulkan/VKUtil.h"
#include <vector>

namespace ngfx {
    class VKFramebuffer : public Framebuffer {
    public:
        struct VKAttachmentInfo {
            VkImageUsageFlags imageUsageFlags;
        };
        struct VKAttachmentDescriptor {
            VKImageView* imageView = nullptr;
            VKAttachmentInfo info;
        };
        void create(VkDevice, VkRenderPass renderPass,
                    const std::vector<VKAttachmentDescriptor> &attachments,
                    uint32_t w, uint32_t h, uint32_t layers = 1);
        virtual ~VKFramebuffer();
        VkFramebuffer v = VK_NULL_HANDLE;
        std::vector<VkImageView> vkAttachments;
        std::vector<VKAttachmentInfo> vkAttachmentInfos;
         VkFramebufferCreateInfo createInfo;
    private:
        VkDevice device;
    };
    VK_CAST(Framebuffer);
}
