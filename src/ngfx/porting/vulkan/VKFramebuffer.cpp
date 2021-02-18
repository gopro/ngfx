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
#include "ngfx/porting/vulkan/VKFramebuffer.h"
#include "ngfx/porting/vulkan/VKDevice.h"
#include "ngfx/porting/vulkan/VKRenderPass.h"
#include "ngfx/porting/vulkan/VKTexture.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"

using namespace ngfx;

void VKFramebuffer::create(VkDevice device, VkRenderPass renderPass, 
        const std::vector<VKAttachmentDescriptor> &attachments, uint32_t w, uint32_t h, uint32_t layers) {
    this->device = device;
    VkResult vkResult;
    this->numAttachments = attachments.size();
    this->w = w; this->h = h; this->layers = layers;
    vkAttachments.resize(attachments.size());
    vkAttachmentInfos.resize(attachments.size());
    for (uint32_t j = 0; j<attachments.size(); j++) {
        vkAttachments[j] = attachments[j].imageView->v;
        vkAttachmentInfos[j] = attachments[j].info;
    }
    createInfo = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, NULL, 0,
        renderPass, uint32_t(vkAttachments.size()), vkAttachments.data(), w, h, layers
    };
    V(vkCreateFramebuffer(device, &createInfo, nullptr, &v));
}

VKFramebuffer::~VKFramebuffer() {
    if (v) VK_TRACE(vkDestroyFramebuffer(device, v, nullptr));
}

Framebuffer* Framebuffer::create(Device* device, RenderPass* renderPass,
        const std::vector<Attachment> &attachments, uint32_t w, uint32_t h, uint32_t layers) {
    VKFramebuffer* vkFramebuffer = new VKFramebuffer();
    vkFramebuffer->attachments = attachments;
    std::vector<VKFramebuffer::VKAttachmentDescriptor> vkAttachments(attachments.size());
    for (int j = 0; j<vkAttachments.size(); j++) {
        auto& attachment = attachments[j];
        auto vkTexture = vk(attachment.texture);
        auto &vkAttachment = vkAttachments[j];
        vkAttachment.info = { VkImageUsageFlags(vkTexture->imageUsageFlags) };
        vkAttachment.imageView = vkTexture->getImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1,
            attachment.level, attachment.layer);
    }
    vkFramebuffer->create(vk(device)->v, vk(renderPass)->v, vkAttachments, w, h, layers);
    return vkFramebuffer;
}
