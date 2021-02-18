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
#include <cstring>
#include <vulkan/vulkan.h>

namespace ngfx {
struct VKImageViewCreateInfo : VkImageViewCreateInfo {
  VKImageViewCreateInfo() {
    sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    pNext = NULL;
    flags = 0;
    image = 0;
    viewType = VK_IMAGE_VIEW_TYPE_2D;
    format = VK_FORMAT_R8G8B8A8_UNORM;
    components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                  VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  };
  VKImageViewCreateInfo(
      VkImage image, VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D,
      VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM,
      VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
      uint32_t baseMipLevel = 0, uint32_t baseArrayLayer = 0) {
    sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    pNext = NULL;
    flags = 0;
    this->image = image;
    viewType = imageViewType;
    format = colorFormat;
    components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                  VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    subresourceRange = {aspectMask, baseMipLevel, mipLevels, baseArrayLayer,
                        arrayLayers};
  };
};
static bool operator==(const VKImageViewCreateInfo &lhs,
                       const VKImageViewCreateInfo &rhs) {
  return lhs.sType == rhs.sType && lhs.pNext == rhs.pNext &&
         lhs.flags == rhs.flags && lhs.image == rhs.image &&
         lhs.viewType == rhs.viewType && lhs.format == rhs.format &&
         lhs.components.a == rhs.components.a &&
         lhs.components.b == rhs.components.b &&
         lhs.components.g == rhs.components.g &&
         lhs.components.r == rhs.components.r &&
         lhs.subresourceRange.aspectMask == rhs.subresourceRange.aspectMask &&
         lhs.subresourceRange.baseArrayLayer ==
             rhs.subresourceRange.baseArrayLayer &&
         lhs.subresourceRange.baseMipLevel ==
             rhs.subresourceRange.baseMipLevel &&
         lhs.subresourceRange.layerCount == rhs.subresourceRange.layerCount &&
         lhs.subresourceRange.levelCount == rhs.subresourceRange.levelCount;
}
} // namespace ngfx
