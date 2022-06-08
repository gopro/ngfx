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
#include "ngfx/porting/vulkan/VKGraphicsPipeline.h"
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/GraphicsContext.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
#include "ngfx/porting/vulkan/VKShaderModule.h"
#include <vector>
using namespace ngfx;

void VKGraphicsPipeline::create(
    VKGraphicsContext *ctx, const State &state,
    const std::vector<VKPipeline::Descriptor> &descriptors,
    const std::vector<VkVertexInputBindingDescription> &vertexInputBindings,
    const std::vector<VkVertexInputAttributeDescription> &vertexInputAttributes,
    const std::vector<VKPipeline::ShaderStage> &shaderStages,
    VkFormat colorFormat) {
  this->device = vk(ctx->device)->v;
  VkResult vkResult;

  inputAssemblyState = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
      state.primitiveTopology, VK_FALSE};

  rasterizationState = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      nullptr,
      0,
      VK_FALSE,
      VK_FALSE,
      state.polygonMode,
      state.cullModeFlags,
      state.frontFace,
      VK_FALSE,
      0.0f,
      0.0f,
      0.0f,
      state.lineWidth};
  blendAttachmentState.resize(state.numColorAttachments);
  for (auto &pState : blendAttachmentState)
    pState = {state.blendEnable,         state.srcColorBlendFactor,
              state.dstColorBlendFactor, state.colorBlendOp,
              state.srcColorBlendFactor, state.dstColorBlendFactor,
              state.alphaBlendOp,        state.colorWriteMask};
  colorBlendState = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                     nullptr,
                     0,
                     VK_FALSE,
                     VK_LOGIC_OP_CLEAR,
                     uint32_t(blendAttachmentState.size()),
                     blendAttachmentState.data(),
                     {0.0f, 0.0f, 0.0f, 0.0f}};
  viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                   nullptr,
                   0,
                   1,
                   nullptr,
                   1,
                   nullptr};
  dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr,
                  0, uint32_t(dynamicStateEnables.size()),
                  dynamicStateEnables.data()};

  depthStencilState = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      nullptr,
      0,
      state.depthTestEnable,
      state.depthWriteEnable,
      state.depthFunc,
      VK_FALSE,
      state.stencilEnable,
      {state.frontStencilFailOp, state.frontStencilPassOp, state.frontStencilDepthFailOp,
       state.frontStencilFunc, state.stencilReadMask, state.stencilWriteMask, state.stencilRef},
      {state.backStencilFailOp, state.backStencilPassOp, state.backStencilDepthFailOp,
       state.backStencilFunc, state.stencilReadMask, state.stencilWriteMask, state.stencilRef},
      0.0f,
      0.0f};

  multisampleState = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                      nullptr,
                      0,
                      VkSampleCountFlagBits(state.numSamples),
                      VK_FALSE,
                      0.0f,
                      nullptr,
                      VK_FALSE,
                      VK_FALSE};

  vertexInputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                      nullptr,
                      0,
                      uint32_t(vertexInputBindings.size()),
                      vertexInputBindings.data(),
                      uint32_t(vertexInputAttributes.size()),
                      vertexInputAttributes.data()};

  vkShaderStages.resize(shaderStages.size());
  for (int j = 0; j < shaderStages.size(); j++) {
    auto &shaderStage = shaderStages[j];
    vkShaderStages[j] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                         nullptr,
                         0,
                         shaderStage.stage,
                         shaderStage.module->v,
                         "main",
                         nullptr};
  }
  descriptorSetLayouts.resize(descriptors.size());
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

  createInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                nullptr,
                0,
                static_cast<uint32_t>(vkShaderStages.size()),
                vkShaderStages.data(),
                &vertexInputState,
                &inputAssemblyState,
                nullptr,
                &viewportState,
                &rasterizationState,
                &multisampleState,
                &depthStencilState,
                &colorBlendState,
                &dynamicState,
                pipelineLayout,
                state.renderPass,
                0,
                0,
                0};
  V(vkCreateGraphicsPipelines(device, ctx->vkPipelineCache.v, 1, &createInfo,
                              nullptr, &v));
}

static std::map<VertexFormat, uint32_t> strideMap = {{VERTEXFORMAT_FLOAT, 4},
                                                     {VERTEXFORMAT_FLOAT2, 8},
                                                     {VERTEXFORMAT_FLOAT3, 12},
                                                     {VERTEXFORMAT_FLOAT4, 16}};
GraphicsPipeline *
GraphicsPipeline::create(GraphicsContext* graphicsContext, const State& state,
    VertexShaderModule* vs, FragmentShaderModule* fs,
    PixelFormat colorFormat, PixelFormat depthFormat,
    std::vector<VertexInputAttributeDescription> vertexAttributes,
    std::set<std::string> instanceAttributes) {
  VKGraphicsPipeline *vkGraphicsPipeline = new VKGraphicsPipeline();
  VKGraphicsPipeline::State vkState = {
      VkPrimitiveTopology(state.primitiveTopology),
      VkPolygonMode(state.polygonMode),
      state.blendEnable,
      VkBlendFactor(state.srcColorBlendFactor),
      VkBlendFactor(state.dstColorBlendFactor),
      VkBlendFactor(state.srcAlphaBlendFactor),
      VkBlendFactor(state.dstAlphaBlendFactor),
      VkBlendOp(state.colorBlendOp),
      VkBlendOp(state.alphaBlendOp),
      VkColorComponentFlags(state.colorWriteMask),
      VkCullModeFlags(state.cullModeFlags),
      VkFrontFace(state.frontFace),
      state.lineWidth,
      state.depthTestEnable,
      state.depthWriteEnable,
      VkCompareOp(state.depthFunc),
      state.stencilEnable,
      state.stencilReadMask,
      state.stencilWriteMask,
      VkStencilOp(state.frontStencilFailOp),
      VkStencilOp(state.frontStencilDepthFailOp),
      VkStencilOp(state.frontStencilPassOp),
      VkCompareOp(state.frontStencilFunc),
      VkStencilOp(state.backStencilFailOp),
      VkStencilOp(state.backStencilDepthFailOp),
      VkStencilOp(state.backStencilPassOp),
      VkCompareOp(state.backStencilFunc),
      state.stencilRef,
      vk(state.renderPass)->v,
      state.numSamples,
      state.numColorAttachments};
  auto &descriptorBindings = vkGraphicsPipeline->descriptorBindings;

  uint32_t numDescriptors =
      glm::max(vs->descriptors.empty() ? 0 : vs->descriptors.back().set + 1,
               fs->descriptors.empty() ? 0 : fs->descriptors.back().set + 1);
  descriptorBindings.resize(numDescriptors);
  std::vector<VKPipeline::Descriptor> vkDescriptors(numDescriptors);
  VKPipelineUtil::parseDescriptors(vs->descriptors, VK_SHADER_STAGE_VERTEX_BIT,
                                   vkDescriptors, descriptorBindings);
  VKPipelineUtil::parseDescriptors(fs->descriptors,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, vkDescriptors,
                                   descriptorBindings);

  std::vector<VkVertexInputAttributeDescription> vkVertexInputAttributes;
  auto &vertexAttributeBindings = vkGraphicsPipeline->vertexAttributeBindings;
  vertexAttributeBindings.resize(vs->attributes.size());
  // TODO: support interleaved vertex bindings
  for (uint32_t j = 0; j < vs->attributes.size(); j++) {
    auto &va = vs->attributes[j];
    uint32_t binding = va.location, offset = vertexAttributes.empty() ? 0 : vertexAttributes[j].offset;
    for (uint32_t k = 0; k < va.count; k++) {
      vkVertexInputAttributes.push_back(
          {va.location + k, binding, VkFormat(va.format), offset});
      offset += va.elementSize;
    }
    vertexAttributeBindings[j] = j;
  }
  std::vector<VKPipeline::ShaderStage> vkShaderStages = {
      {VK_SHADER_STAGE_VERTEX_BIT, (VKVertexShaderModule *)vs},
      {VK_SHADER_STAGE_FRAGMENT_BIT, (VKFragmentShaderModule *)fs}};
  std::vector<VkVertexInputBindingDescription> vkVertexInputBindings(
      vs->attributes.size());
  for (uint32_t j = 0; j < vs->attributes.size(); j++) {
    auto &attr = vs->attributes[j];
    uint32_t binding = attr.location;
    VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    if (instanceAttributes.find(attr.name) != instanceAttributes.end())
      inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    vkVertexInputBindings[j] = {binding, attr.elementSize * attr.count,
                                inputRate};
  }
  vkGraphicsPipeline->create(vk(graphicsContext), vkState, vkDescriptors,
                             vkVertexInputBindings, vkVertexInputAttributes,
                             vkShaderStages, VkFormat(colorFormat));
  return vkGraphicsPipeline;
}
