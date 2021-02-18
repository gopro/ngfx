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
#include "ngfx/porting/vulkan/VKDevice.h"
#include "ngfx/porting/vulkan/VKImageViewCreateInfo.h"
#include <vulkan/vulkan.h>

namespace ngfx {
class VKImageView {
public:
  void create(VkDevice device, VkImage image,
              VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D,
              VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM,
              VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
              uint32_t baseMipLevel = 0, uint32_t baseArrayLayer = 0);
  void create(VkDevice device, VKImageViewCreateInfo &createInfo);
  virtual ~VKImageView();
  VkImageView v = VK_NULL_HANDLE;
  VKImageViewCreateInfo createInfo;

private:
  VkDevice device;
};
} // namespace ngfx
