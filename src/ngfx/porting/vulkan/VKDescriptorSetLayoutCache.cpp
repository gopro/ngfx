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
#include "ngfx/porting/vulkan/VKDescriptorSetLayoutCache.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
using namespace ngfx;

void VKDescriptorSetLayoutCache::create(VkDevice device) {
    this->device = device;
    initDescriptorSetLayout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    initDescriptorSetLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    initDescriptorSetLayout(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    initDescriptorSetLayout(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

VKDescriptorSetLayoutCache::~VKDescriptorSetLayoutCache() {
    for (const auto &it : cache) {
        VK_TRACE(vkDestroyDescriptorSetLayout(device, it.second.layout, nullptr));
    }
}

VkDescriptorSetLayout VKDescriptorSetLayoutCache::get(VkDescriptorType type, VkShaderStageFlags stageFlags) {
    return cache.at(type).layout;
}

void VKDescriptorSetLayoutCache::initDescriptorSetLayout(VkDescriptorType descriptorType) {
    VkResult vkResult;
    VKDescriptorSetLayoutData data;
    data.layoutBinding = {
            0, descriptorType, 1,
            VK_SHADER_STAGE_ALL, nullptr
    };
    data.createInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 1, &data.layoutBinding
    };
    V(vkCreateDescriptorSetLayout(device, &data.createInfo, nullptr, &data.layout));
    cache[descriptorType] = std::move(data);
}
