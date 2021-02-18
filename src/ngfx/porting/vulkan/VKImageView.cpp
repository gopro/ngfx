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
#include "ngfx/porting/vulkan/VKImageView.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
using namespace ngfx;

void VKImageView::create(VkDevice device, VkImage image, VkImageViewType imageViewType,
        VkFormat colorFormat, VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers,
        uint32_t baseMipLevel, uint32_t baseArrayLayer) {
    VKImageViewCreateInfo p(image, imageViewType, colorFormat, aspectMask, mipLevels, arrayLayers, baseMipLevel, baseArrayLayer);
    create(device, p);
}

void VKImageView::create(VkDevice device, VKImageViewCreateInfo &createInfo) {
    VkResult vkResult;
    this->device = device;
    this->createInfo = createInfo;
    V(vkCreateImageView(device, &createInfo, nullptr, &v));
}

VKImageView::~VKImageView() {
    if (v) VK_TRACE(vkDestroyImageView(device, v, nullptr));
}
