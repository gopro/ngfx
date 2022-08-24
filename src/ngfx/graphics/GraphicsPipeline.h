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
#include "ngfx/graphics/BlendUtil.h"
#include <set>
#include <vector>

namespace ngfx {
class GraphicsContext;

/** \struct BlendParams
 *
 *  This struct defines blend parameters and operations
 */
struct BlendParams {
    BlendFactor srcColorBlendFactor = BLEND_FACTOR_SRC_ALPHA;
    BlendFactor dstColorBlendFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    BlendFactor srcAlphaBlendFactor = BLEND_FACTOR_SRC_ALPHA;
    BlendFactor dstAlphaBlendFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    BlendOp colorBlendOp = BLEND_OP_ADD, alphaBlendOp = BLEND_OP_ADD;
    size_t key();
};

/** \struct StencilParams
 *
 *  This struct defines stencil parameters and operations
 */
struct StencilParams {
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
    size_t key();
};

/** \class GraphicsPipeline
 *
 *  This class provides the a platform abstraction API for programming the 
 *  graphics pipeline stages including the input assembly, shader stages 
 *  (vertex, fragment, compute, etc), rasterizer, multisampling, blending, 
 *  stencil, viewport, scissor rect, etc.
 */

class GraphicsPipeline : public Pipeline {
public:
  /** \struct State
   *
   *  The graphics pipeline state.  This provides default parameter values.  
      The user can just override specific parameters instead of redefining all the parameters */
  struct State {
    /** The input type (e.g. triangles, triangle list, lines, etc. )*/
    PrimitiveTopology primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    /** The polygon drawing mode (e.g. fill or wireframe) */
    PolygonMode polygonMode = POLYGON_MODE_FILL;
    /** A helper function for setting the blend parameters for a given porter duff blend mode */
    inline void setBlendMode(BlendMode mode) {
        blendEnable = true;
        blendParams = BlendUtil::getBlendParams(mode);
    }
    /** Enable / disable blending */
    bool blendEnable = false;
    /** The blend params */
    BlendParams blendParams;
    /** The color write mask (which color bits are written to the color buffer) */
    uint8_t colorWriteMask = COLOR_COMPONENT_R_BIT | COLOR_COMPONENT_G_BIT |
                             COLOR_COMPONENT_B_BIT | COLOR_COMPONENT_A_BIT;
    /** The cull mode (e.g. front, or backface culling )*/
    CullModeFlags cullModeFlags = CULL_MODE_BACK_BIT;
    FrontFace frontFace = FRONT_FACE_COUNTER_CLOCKWISE;
    /** The line width */
    float lineWidth = 1.0f;
    /** Enable depth test */
    bool depthTestEnable = false,
    /** Enable depth write */
        depthWriteEnable = false;
    /** The depth comparision function that filters which values are written to the depth buffer */
    CompareOp depthFunc = COMPARE_OP_LESS;
    /** Enable stencil test */
    bool stencilEnable = false;
    /** The stencil parameters */
    StencilParams stencilParams;
    RenderPass* renderPass = nullptr;
    /** The number of samples used for multisampling.
     *  When this value is one, no multisampling is used. */
    uint32_t numSamples = 1,
    /** The number of color attachments (supports multiple render targets) */
        numColorAttachments = 1;
    /** Create a unique key associated with the pipeline state */
    size_t key();
  };
  struct Descriptor {
    DescriptorType type;
    ShaderStageFlags stageFlags = SHADER_STAGE_ALL;
  };
  struct VertexInputAttributeDescription {
      VertexShaderModule::AttributeDescription* v = nullptr;
      int offset = 0;
  };
  /** Create graphics pipeline */
  static GraphicsPipeline *
  create(GraphicsContext *graphicsContext, const State &state,
         VertexShaderModule *vs, FragmentShaderModule *fs,
         PixelFormat colorFormat, PixelFormat depthStencilFormat,
         std::vector<VertexInputAttributeDescription> vertexAttributes = {},
         std::set<std::string> instanceAttributes = {});
  virtual ~GraphicsPipeline() {}
  /** Get the binding indices associated with the descriptors and vertex attributes.
  *   The user passes an array of pointers, one pointer for each descriptor and vertex attribute.
  *   The user should pass the pointers associated with the descriptors and vertex attributes in the same order 
  *   that they're declared in the shader.
  *   @param pDescriptorBindings This return parameter contains the descriptor binding indices.
  *   @param pVertexAttribBindings This return parameter contains the vertex attribute binding indices. */
  void getBindings(std::vector<uint32_t *> pDescriptorBindings,
                   std::vector<uint32_t *> pVertexAttribBindings);
  std::vector<uint32_t> descriptorBindings, vertexAttributeBindings;
};
}; // namespace ngfx
