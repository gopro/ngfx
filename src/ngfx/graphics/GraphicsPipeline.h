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
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/graphics/GraphicsCore.h"
#include "ngfx/graphics/Pipeline.h"
#include "ngfx/graphics/RenderPass.h"
#include "ngfx/graphics/ShaderModule.h"
#include <set>
#include <vector>

namespace ngfx {
class GraphicsContext;
class GraphicsPipeline : public Pipeline {
public:
  struct State {
    PrimitiveTopology primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PolygonMode polygonMode = POLYGON_MODE_FILL;
    bool blendEnable = false;
    BlendFactor srcColorBlendFactor = BLEND_FACTOR_SRC_ALPHA;
    BlendFactor dstColorBlendFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    BlendFactor srcAlphaBlendFactor = BLEND_FACTOR_SRC_ALPHA;
    BlendFactor dstAlphaBlendFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    BlendOp colorBlendOp = BLEND_OP_ADD, alphaBlendOp = BLEND_OP_ADD;
    uint8_t colorWriteMask = COLOR_COMPONENT_R_BIT | COLOR_COMPONENT_G_BIT |
                             COLOR_COMPONENT_B_BIT | COLOR_COMPONENT_A_BIT;
    CullModeFlags cullModeFlags = CULL_MODE_BACK_BIT;
    FrontFace frontFace = FRONT_FACE_COUNTER_CLOCKWISE;
    float lineWidth = 1.0f;
    bool depthTestEnable = false, depthWriteEnable = false;
    CompareOp depthFunc = COMPARE_OP_LESS;
    bool stencilEnable = false;
    uint8_t stencilReadMask = DEFAULT_STENCIL_READ_MASK;
    uint8_t stencilWriteMask = DEFAULT_STENCIL_WRITE_MASK;
    StencilOp frontStencilFailOp = STENCIL_OP_KEEP;
    StencilOp frontStencilDepthFailOp = STENCIL_OP_KEEP;
    StencilOp frontStencilPassOp = STENCIL_OP_KEEP;
    CompareOp frontStencilFunc = COMPARE_OP_ALWAYS;
    StencilOp backStencilFailOp = STENCIL_OP_KEEP;
    StencilOp backStencilDepthFailOp = STENCIL_OP_KEEP;
    StencilOp backStencilPassOp = STENCIL_OP_KEEP;
    CompareOp backStencilFunc = COMPARE_OP_ALWAYS;
    uint32_t stencilRef = 0;
    RenderPass* renderPass = nullptr;
    uint32_t numSamples = 1, numColorAttachments = 1;
  };
  struct Descriptor {
    DescriptorType type;
    ShaderStageFlags stageFlags = SHADER_STAGE_ALL;
  };
  struct VertexInputAttributeDescription {
      VertexShaderModule::AttributeDescription* v = nullptr;
      int offset = 0;
  };
  static GraphicsPipeline *
  create(GraphicsContext *graphicsContext, const State &state,
         VertexShaderModule *vs, FragmentShaderModule *fs,
         PixelFormat colorFormat, PixelFormat depthStencilFormat,
         std::vector<VertexInputAttributeDescription> vertexAttributes = {},
         std::set<std::string> instanceAttributes = {});
  virtual ~GraphicsPipeline() {}
  void getBindings(std::vector<uint32_t *> pDescriptorBindings,
                   std::vector<uint32_t *> pVertexAttribBindings);
  std::vector<uint32_t> descriptorBindings, vertexAttributeBindings;
};
}; // namespace ngfx
