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
#include "ngfx/graphics/GraphicsPipeline.h"
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
#include "ngfx/porting/vulkan/VKPipeline.h"
#include "ngfx/porting/vulkan/VKShaderModule.h"
#include "ngfx/porting/vulkan/VKUtil.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace ngfx {
class VKGraphicsPipeline : public GraphicsPipeline, public VKPipeline {
public:
  struct State {
    VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkBool32 blendEnable = VK_FALSE;
    VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp colorBlendOp = VK_BLEND_OP_ADD, alphaBlendOp = VK_BLEND_OP_ADD;
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkCullModeFlags cullModeFlags = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    float lineWidth = 1.0f;
    VkBool32 depthTestEnable = VK_FALSE, depthWriteEnable = VK_FALSE;
    VkCompareOp depthFunc;
    bool stencilEnable = false;
    uint32_t stencilReadMask = DEFAULT_STENCIL_READ_MASK;
    uint32_t stencilWriteMask = DEFAULT_STENCIL_WRITE_MASK;
    VkStencilOp frontStencilFailOp = VK_STENCIL_OP_KEEP;
    VkStencilOp frontStencilDepthFailOp = VK_STENCIL_OP_KEEP;
    VkStencilOp frontStencilPassOp = VK_STENCIL_OP_KEEP;
    VkCompareOp frontStencilFunc = VK_COMPARE_OP_ALWAYS;
    VkStencilOp backStencilFailOp = VK_STENCIL_OP_KEEP;
    VkStencilOp backStencilDepthFailOp = VK_STENCIL_OP_KEEP;
    VkStencilOp backStencilPassOp = VK_STENCIL_OP_KEEP;
    VkCompareOp backStencilFunc = VK_COMPARE_OP_ALWAYS;
    uint32_t stencilRef = 0;
    VkRenderPass renderPass;
    uint32_t numSamples = 1, numColorAttachments = 1;
  };

  void create(
      VKGraphicsContext *ctx, const State &state,
      const std::vector<VKPipeline::Descriptor> &descriptors,
      const std::vector<VkVertexInputBindingDescription> &vertexInputBindings,
      const std::vector<VkVertexInputAttributeDescription>
          &vertexInputAttributes,
      const std::vector<VKPipeline::ShaderStage> &shaderStages,
      VkFormat colorFormat);
  virtual ~VKGraphicsPipeline() {}

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
  VkPipelineRasterizationStateCreateInfo rasterizationState;
  std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentState;
  VkPipelineColorBlendStateCreateInfo colorBlendState;
  VkPipelineViewportStateCreateInfo viewportState;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicState;
  VkPipelineDepthStencilStateCreateInfo depthStencilState;
  VkPipelineMultisampleStateCreateInfo multisampleState;
  VkPipelineVertexInputStateCreateInfo vertexInputState;
  std::vector<VkPipelineShaderStageCreateInfo> vkShaderStages;
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  VkGraphicsPipelineCreateInfo createInfo;
};
VK_CAST(GraphicsPipeline);
}; // namespace ngfx
