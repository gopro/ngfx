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
#include <vulkan/vulkan.h>

namespace ngfx {
struct VKSamplerCreateInfo : VkSamplerCreateInfo {
  VKSamplerCreateInfo() {
    sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    pNext = VK_NULL_HANDLE;
    flags = 0;
    magFilter = VK_FILTER_NEAREST;
    minFilter = VK_FILTER_NEAREST;
    mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    mipLodBias = 0.0f;
    anisotropyEnable = VK_FALSE;
    maxAnisotropy = 0.0f;
    compareEnable = VK_FALSE;
    compareOp = VK_COMPARE_OP_NEVER;
    minLod = 0.0f;
    maxLod = 1.0f;
    borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    unnormalizedCoordinates = VK_FALSE;
  }
};
} // namespace ngfx
