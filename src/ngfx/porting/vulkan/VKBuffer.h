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
#include "ngfx/graphics/Buffer.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/porting/vulkan/VKDevice.h"

namespace ngfx {
class VKGraphicsContext;

class VKBuffer : public Buffer {
public:
  void create(VKGraphicsContext *ctx, const void *data, uint32_t size,
              VkBufferUsageFlags bufferUsageFlags,
              VkMemoryPropertyFlags memoryPropertyFlags =
                  VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
  virtual ~VKBuffer();
  void *map() override;
  void unmap() override;
  void upload(const void *data, uint32_t size, uint32_t offset = 0) override;
  void download(void *data, uint32_t size, uint32_t offset = 0) override;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkBuffer v = VK_NULL_HANDLE;
  VkBufferCreateInfo createInfo;
  VkMemoryAllocateInfo allocInfo;
  const VkDescriptorSet &getUboDescriptorSet(ShaderStageFlags shaderStageFlags);
  const VkDescriptorSet &
  getSsboDescriptorSet(ShaderStageFlags shaderStageFlags);
  VkDescriptorSet uboDescriptorSet = 0, ssboDescriptorSet = 0;

protected:
  void createBuffer(const void *data, uint32_t size,
                    VkBufferUsageFlags bufferUsageFlags);
  void createMemory(VkMemoryPropertyFlags memoryPropertyFlags);
  void initDescriptorSet(VkDescriptorPool descriptorPool,
                         VkDescriptorSetLayout descriptorSetLayout,
                         VkDescriptorType descriptorType,
                         VkDescriptorSet &descriptorSet);
  VKGraphicsContext *ctx;
  VkMemoryRequirements memReqs;
};
VK_CAST(Buffer);
} // namespace ngfx
