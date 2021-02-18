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
namespace ngfx {
struct VKImageCreateInfo : VkImageCreateInfo {
  VKImageCreateInfo() {
    sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    pNext = nullptr;
    flags = 0;
    imageType = VK_IMAGE_TYPE_2D;
    format = VK_FORMAT_R8G8B8A8_UNORM;
    extent = {1, 1, 1};
    mipLevels = 1;
    arrayLayers = 1;
    samples = VK_SAMPLE_COUNT_1_BIT;
    tiling = VK_IMAGE_TILING_OPTIMAL;
    usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    queueFamilyIndexCount = 0;
    pQueueFamilyIndices = nullptr;
    initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  }
};
} // namespace ngfx