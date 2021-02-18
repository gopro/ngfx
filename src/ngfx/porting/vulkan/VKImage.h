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
#include "ngfx/porting/vulkan/VKImageCreateInfo.h"
#include <vulkan/vulkan.h>

namespace ngfx {
class VKImage {
public:
  void create(VKDevice *vkDevice, VkExtent3D extent,
              VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
              VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
              VkImageType imageType = VK_IMAGE_TYPE_2D, uint32_t mipLevels = 1,
              uint32_t arrayLayers = 1, uint32_t numSamples = 1,
              VkImageCreateFlags imageCreateFlags = 0,
              VkMemoryPropertyFlags memoryPropertyFlags =
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  void create(VKDevice *vkDevice, const VKImageCreateInfo &createInfo,
              VkMemoryPropertyFlags memoryPropertyFlags =
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  void changeLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout,
                    VkImageAspectFlags dstAccessMask,
                    VkPipelineStageFlags dstStageMask,
                    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    uint32_t baseMipLevel = 0, uint32_t levelCount = 1,
                    uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);
  virtual ~VKImage();
  VkImage v = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  std::vector<VkImageLayout> imageLayout;
  std::vector<VkAccessFlags> accessMask;
  std::vector<VkPipelineStageFlags> stageMask;
  VKImageCreateInfo createInfo;

private:
  VkDevice device;
};
} // namespace ngfx
