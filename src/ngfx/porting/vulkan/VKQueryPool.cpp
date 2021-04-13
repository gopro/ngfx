/*
 * Copyright 2021 GoPro Inc.
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
#include "ngfx/porting/vulkan/VKQueryPool.h"
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
using namespace ngfx;

void VKQueryPool::create(VKGraphicsContext *ctx, VkQueryType queryType, uint32_t queryCount) {
    VkResult vkResult;
    const VkQueryPoolCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, nullptr, 0,
        queryType, queryCount, 0
    };
    V(vkCreateQueryPool(vk(ctx->device)->v, &createInfo, nullptr, &v));
}

VKQueryPool::~VKQueryPool() {
    vkDestroyQueryPool(vk(ctx->device)->v, v, nullptr);
}

