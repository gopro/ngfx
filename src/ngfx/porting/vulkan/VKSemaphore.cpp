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
#include "ngfx/porting/vulkan/VKSemaphore.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
using namespace ngfx;

void VKSemaphore::create(VkDevice device) {
  this->device = device;
  VkResult vkResult;
  createInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
  V(vkCreateSemaphore(device, &createInfo, nullptr, &v));
}

VKSemaphore::~VKSemaphore() {
  if (v)
    VK_TRACE(vkDestroySemaphore(device, v, nullptr));
}

uint64_t VKSemaphore::wait() {
  /*VkResult vkResult;
  uint64_t value;
  VkSemaphoreWaitInfo waitInfo = {
      VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO, nullptr, 0,
      1, &v, &value
  };
  V(vkWaitSemaphores(device, &waitInfo, UINT64_MAX));
  return value;*/
  return 0;
}

void VKSemaphore::signal(uint64_t value) {
  /*VkResult vkResult;
  VkSemaphoreSignalInfo signalInfo = {
  VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO, nullptr, v, value
  };
  V(vkSignalSemaphore(device, &signalInfo));*/
}
