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
#include "ngfx/porting/vulkan/VKFence.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
using namespace ngfx;

void VKFence::create(VkDevice device, VkFenceCreateFlags flags) {
    this->device = device;
    VkResult vkResult;
    createInfo = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, flags
    };
    V(vkCreateFence(device, &createInfo, nullptr, &v));
}

VKFence::~VKFence() {
    if (v) VK_TRACE(vkDestroyFence(device, v, nullptr));
}

void VKFence::wait() {
    VkResult vkResult;
    V(vkWaitForFences(device, 1, &v, VK_TRUE, UINT64_MAX));
}

void VKFence::reset() {
    VkResult vkResult;
    V(vkResetFences(device, 1, &v));
}
