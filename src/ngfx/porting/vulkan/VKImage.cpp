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
#include "ngfx/porting/vulkan/VKImage.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
using namespace ngfx;

void VKImage::create(VKDevice *vkDevice, VkExtent3D extent, VkFormat format,
                     VkImageUsageFlags usage, VkImageType imageType,
                     uint32_t mipLevels, uint32_t arrayLayers,
                     uint32_t numSamples, VkImageCreateFlags imageCreateFlags,
                     VkMemoryPropertyFlags memoryPropertyFlags) {
  VKImageCreateInfo p;
  p.format = format;
  p.extent = extent;
  p.usage = usage;
  p.imageType = imageType;
  p.mipLevels = mipLevels;
  p.arrayLayers = arrayLayers;
  p.flags = imageCreateFlags;
  p.samples = VkSampleCountFlagBits(numSamples);
  create(vkDevice, p, memoryPropertyFlags);
}

void VKImage::create(VKDevice *vkDevice, const VKImageCreateInfo &createInfo,
                     VkMemoryPropertyFlags memoryPropertyFlags) {
  this->device = vkDevice->v;
  this->createInfo = createInfo;
  VkResult vkResult;
  V(vkCreateImage(device, &createInfo, nullptr, &v));
  uint32_t mipLevels = createInfo.mipLevels,
           arrayLayers = createInfo.arrayLayers;
  imageLayout.resize(mipLevels * arrayLayers);
  accessMask.resize(mipLevels * arrayLayers);
  stageMask.resize(mipLevels * arrayLayers);
  for (uint32_t j = 0; j < (mipLevels * arrayLayers); j++) {
    imageLayout[j] = createInfo.initialLayout;
    accessMask[j] = 0;
    stageMask[j] = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  }
  VkMemoryRequirements memReqs = {};
  vkGetImageMemoryRequirements(device, v, &memReqs);

  VkMemoryAllocateInfo memAllloc = {};
  memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllloc.allocationSize = memReqs.size;
  memAllloc.memoryTypeIndex = vkDevice->vkPhysicalDevice->getMemoryType(
      memReqs.memoryTypeBits, memoryPropertyFlags);
  V(vkAllocateMemory(device, &memAllloc, nullptr, &memory));
  V(vkBindImageMemory(device, v, memory, 0));
}

void VKImage::changeLayout(VkCommandBuffer commandBuffer,
                           VkImageLayout newLayout, VkAccessFlags dstAccessMask,
                           VkPipelineStageFlags dstStageMask,
                           VkImageAspectFlags aspectMask, uint32_t baseMipLevel,
                           uint32_t levelCount, uint32_t baseArrayLayer,
                           uint32_t layerCount) {
  uint32_t baseIndex = baseArrayLayer * createInfo.mipLevels + baseMipLevel;
  VkImageLayout srcImageLayout = imageLayout[baseIndex];
  if (srcImageLayout == newLayout) {
    return;
  }
  VkAccessFlags srcAccessMask = accessMask[baseIndex];
  VkPipelineStageFlags srcStageMask = stageMask[baseIndex];
  VkImageSubresourceRange subresourceRange = {
      aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount};
  VkImageMemoryBarrier imageMemoryBarrier = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      nullptr,
      srcAccessMask,
      dstAccessMask,
      srcImageLayout,
      newLayout,
      VK_QUEUE_FAMILY_IGNORED,
      VK_QUEUE_FAMILY_IGNORED,
      v,
      subresourceRange};
  VK_TRACE(vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0,
                                nullptr, 0, nullptr, 1, &imageMemoryBarrier));
  for (uint32_t layer = baseArrayLayer; layer < (baseArrayLayer + layerCount);
       layer++) {
    for (uint32_t level = baseMipLevel; level < (baseMipLevel + levelCount);
         level++) {
      uint32_t index = layer * createInfo.mipLevels + level;
      imageLayout[index] = newLayout;
      accessMask[index] = dstAccessMask;
      stageMask[index] = dstStageMask;
    }
  }
}

VKImage::~VKImage() {
  if (v)
    VK_TRACE(vkDestroyImage(device, v, nullptr));
  if (memory)
    VK_TRACE(vkFreeMemory(device, memory, nullptr));
}
