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
#include "ngfx/porting/vulkan/VKCommandBuffer.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
using namespace ngfx;

void VKCommandBuffer::create(VkDevice device, VkCommandPool cmdPool,
                             VkCommandBufferLevel level) {
  this->device = device;
  this->cmdPool = cmdPool;
  VkResult vkResult;
  allocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
                  cmdPool, level, 1};

  V(vkAllocateCommandBuffers(device, &allocateInfo, &v));
}

VKCommandBuffer::~VKCommandBuffer() {
  if (v)
    VK_TRACE(vkFreeCommandBuffers(device, cmdPool, 1, &v));
}

void VKCommandBuffer::begin() {
  VkResult vkResult;
  VkCommandBufferBeginInfo cmdBufferBeginInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, nullptr};
  V(vkBeginCommandBuffer(v, &cmdBufferBeginInfo));
}
void VKCommandBuffer::end() {
  VkResult vkResult;
  V(vkEndCommandBuffer(v));
}
