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
#include "ngfx/porting/vulkan/VKComputePipeline.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
using namespace ngfx;

void VKComputePipeline::create(
    VKGraphicsContext *ctx,
    const std::vector<VKPipeline::Descriptor> &descriptors,
    VkShaderModule shaderModule) {
  VkResult vkResult;
  this->device = ctx->vkDevice.v;
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts(descriptors.size());
  for (int j = 0; j < descriptors.size(); j++) {
    auto &descriptor = descriptors[j];
    descriptorSetLayouts[j] = ctx->vkDescriptorSetLayoutCache.get(
        descriptor.type, descriptor.stageFlags);
  }
  pipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                              nullptr,
                              0,
                              uint32_t(descriptorSetLayouts.size()),
                              descriptorSetLayouts.data(),
                              0,
                              nullptr};
  V(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr,
                           &pipelineLayout));

  shaderStageCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                           nullptr,
                           0,
                           VK_SHADER_STAGE_COMPUTE_BIT,
                           shaderModule,
                           "main",
                           nullptr};

  createInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                nullptr,
                0,
                shaderStageCreateInfo,
                pipelineLayout,
                0,
                0};
  V(vkCreateComputePipelines(device, ctx->vkPipelineCache.v, 1, &createInfo,
                             nullptr, &v));
}

ComputePipeline *ComputePipeline::create(GraphicsContext *graphicsContext,
                                         ComputeShaderModule *cs) {
  VKComputePipeline *vkComputePipeline = new VKComputePipeline();
  uint32_t numDescriptors =
      cs->descriptors.empty() ? 0 : cs->descriptors.back().set + 1;
  auto &descriptorBindings = vkComputePipeline->descriptorBindings;
  descriptorBindings.resize(numDescriptors);
  std::vector<VKPipeline::Descriptor> vkDescriptors(numDescriptors);
  VKPipelineUtil::parseDescriptors(cs->descriptors, VK_SHADER_STAGE_COMPUTE_BIT,
                                   vkDescriptors, descriptorBindings);
  vkComputePipeline->create(vk(graphicsContext), vkDescriptors, vk(cs)->v);
  return vkComputePipeline;
}
