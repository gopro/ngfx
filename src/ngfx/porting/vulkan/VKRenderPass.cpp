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
#include "ngfx/porting/vulkan/VKRenderPass.h"
#include "ngfx/porting/vulkan/VKCommandBuffer.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/porting/vulkan/VKTexture.h"
#include <vector>
using namespace ngfx;

void VKRenderPass::create(
    VkDevice device,
    const std::vector<VkAttachmentDescription> &pAttachmentsDesc,
    const std::vector<VkSubpassDescription> &pSubpassesDesc,
    const std::vector<VkSubpassDependency> &pDependenciesDesc) {
  VkResult vkResult;
  this->device = device;
  this->attachmentsDesc = pAttachmentsDesc;
  this->subpassesDesc = pSubpassesDesc;
  this->dependencies = pDependenciesDesc;
  createInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                nullptr,
                0,
                uint32_t(attachmentsDesc.size()),
                attachmentsDesc.data(),
                uint32_t(subpassesDesc.size()),
                subpassesDesc.data(),
                uint32_t(dependencies.size()),
                dependencies.data()};
  V(vkCreateRenderPass(device, &createInfo, nullptr, &v));
}

VKRenderPass::~VKRenderPass() {
  if (v)
    VK_TRACE(vkDestroyRenderPass(device, v, nullptr));
}
