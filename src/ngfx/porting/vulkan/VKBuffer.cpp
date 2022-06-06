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
#include "ngfx/porting/vulkan/VKBuffer.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
#include <cstring>
using namespace ngfx;

void VKBuffer::create(VKGraphicsContext *ctx, const void *data, uint32_t size,
                      VkBufferUsageFlags bufferUsageFlags,
                      VkMemoryPropertyFlags memoryPropertyFlags) {
  this->ctx = ctx;
  this->size = size;
  createBuffer(data, size, bufferUsageFlags);
  createMemory(memoryPropertyFlags);
  if (data)
    upload(data, size, 0);
}

const VkDescriptorSet &
VKBuffer::getUboDescriptorSet(ShaderStageFlags shaderStageFlags) {
  if (!uboDescriptorSet) {
    VkDescriptorPool &descriptorPool = ctx->vkDescriptorPool;
    auto &bufferUsageFlags = createInfo.usage;
    if (!(bufferUsageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT))
      NGFX_ERR("incorrect buffer usage flags");
    auto descriptorSetLayout = ctx->vkDescriptorSetLayoutCache.get(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, shaderStageFlags);
    initDescriptorSet(descriptorPool, descriptorSetLayout,
                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uboDescriptorSet);
  }
  return uboDescriptorSet;
}

const VkDescriptorSet &
VKBuffer::getSsboDescriptorSet(ShaderStageFlags shaderStageFlags) {
  if (!ssboDescriptorSet) {
    VkDescriptorPool &descriptorPool = ctx->vkDescriptorPool;
    auto &bufferUsageFlags = createInfo.usage;
    if (!(bufferUsageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
      NGFX_ERR("incorrect buffer usage flags");
    auto descriptorSetLayout = ctx->vkDescriptorSetLayoutCache.get(
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, shaderStageFlags);
    initDescriptorSet(descriptorPool, descriptorSetLayout,
                      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, ssboDescriptorSet);
  }
  return ssboDescriptorSet;
}

void VKBuffer::initDescriptorSet(VkDescriptorPool descriptorPool,
                                 VkDescriptorSetLayout descriptorSetLayout,
                                 VkDescriptorType descriptorType,
                                 VkDescriptorSet &descriptorSet) {
  VkResult vkResult;
  auto device = ctx->vkDevice.v;
  VkDescriptorSetAllocateInfo allocInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, descriptorPool,
      1, &descriptorSetLayout};
  V(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
  VkDescriptorBufferInfo descriptorBufferInfo = {v, 0, size};
  VkWriteDescriptorSet writeDescriptorSet = {
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      nullptr,
      descriptorSet,
      0,
      0,
      1,
      descriptorType,
      nullptr,
      &descriptorBufferInfo,
      nullptr};
  VK_TRACE(vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr));
}

void VKBuffer::upload(const void *data, uint32_t size, uint32_t offset) {
  uint8_t *dst = (uint8_t *)map();
  memcpy(dst + offset, data, size);
  unmap();
}

void VKBuffer::download(void *dst, uint32_t size, uint32_t offset) {
  uint8_t *src = (uint8_t *)map();
  memcpy(dst, src + offset, size);
  unmap();
}

VKBuffer::~VKBuffer() {
  auto device = ctx->vkDevice.v;
  if (v)
    VK_TRACE(vkDestroyBuffer(device, v, nullptr));
  if (memory)
    VK_TRACE(vkFreeMemory(device, memory, nullptr));
}

void VKBuffer::createBuffer(const void *data, uint32_t size,
                            VkBufferUsageFlags bufferUsageFlags) {
  VkResult vkResult;
  auto device = ctx->vkDevice.v;
  createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                nullptr,
                0,
                VkDeviceSize(size),
                bufferUsageFlags,
                VK_SHARING_MODE_EXCLUSIVE,
                0,
                0};

  V(vkCreateBuffer(device, &createInfo, nullptr, &v));
}

void VKBuffer::createMemory(VkMemoryPropertyFlags memoryPropertyFlags) {
  VkResult vkResult;
  auto device = ctx->vkDevice.v;

  VK_TRACE(vkGetBufferMemoryRequirements(device, v, &memReqs));
  auto physicalDevice = &ctx->vkPhysicalDevice;
  uint32_t memoryTypeIndex = physicalDevice->getMemoryType(
      memReqs.memoryTypeBits, memoryPropertyFlags);
  allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, memReqs.size,
               memoryTypeIndex};
  V(vkAllocateMemory(device, &allocInfo, nullptr, &memory));
  V(vkBindBufferMemory(device, v, memory, 0));
}

void *VKBuffer::map() {
  if (data)
    return data;
  VkResult vkResult;
  auto device = ctx->vkDevice.v;
  V(vkMapMemory(device, memory, 0, size, 0, &data));
  return data;
}

void VKBuffer::unmap() {
  if (!data)
    return;
  auto device = ctx->vkDevice.v;
  VK_TRACE(vkUnmapMemory(device, memory));
  data = nullptr;
}

Buffer *Buffer::create(GraphicsContext *ctx, const void *data, uint32_t size,
                       BufferUsageFlags usageFlags) {
  VKBuffer *vkBuffer = new VKBuffer();
  vkBuffer->create(vk(ctx), data, size, usageFlags);
  return vkBuffer;
}
