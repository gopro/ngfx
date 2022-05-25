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
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
#include "ngfx/porting/d3d/D3DPipeline.h"
#include "ngfx/porting/d3d/D3DUtil.h"

namespace ngfx {
class D3DGraphicsPipeline : public GraphicsPipeline, public D3DPipeline {
public:
  struct State {
    D3D12_PRIMITIVE_TOPOLOGY primitiveTopology =
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID;
    bool blendEnable = false;
    D3D12_BLEND blendSrcColorFactor = D3D12_BLEND_SRC_ALPHA;
    D3D12_BLEND blendDstColorFactor = D3D12_BLEND_INV_SRC_ALPHA;
    D3D12_BLEND_OP blendColorOp = D3D12_BLEND_OP_ADD;
    D3D12_BLEND blendSrcAlphaFactor = D3D12_BLEND_SRC_ALPHA;
    D3D12_BLEND blendDstAlphaFactor = D3D12_BLEND_INV_SRC_ALPHA;
    D3D12_BLEND_OP blendAlphaOp = D3D12_BLEND_OP_ADD;
    UINT8 colorWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK;
    bool frontFaceCounterClockwise = false;
    float lineWidth = 1.0f;
    bool depthTestEnable = false, depthWriteEnable = false;
    D3D12_COMPARISON_FUNC depthFunc = D3D12_COMPARISON_FUNC_LESS;
    bool stencilEnable = false;
    UINT8 stencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    UINT8 stencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    D3D12_STENCIL_OP frontStencilFailOp = D3D12_STENCIL_OP_KEEP;
    D3D12_STENCIL_OP frontStencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    D3D12_STENCIL_OP frontStencilPassOp = D3D12_STENCIL_OP_KEEP;
    D3D12_COMPARISON_FUNC frontStencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    D3D12_STENCIL_OP backStencilFailOp = D3D12_STENCIL_OP_KEEP;
    D3D12_STENCIL_OP backStencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    D3D12_STENCIL_OP backStencilPassOp = D3D12_STENCIL_OP_KEEP;
    D3D12_COMPARISON_FUNC backStencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    uint32_t stencilRef = 0;
    D3DRenderPass *renderPass = nullptr;
    uint32_t numSamples = 1, numColorAttachments = 1;
  };
  struct Shaders {
    D3D12_SHADER_BYTECODE VS{}, PS{}, DS{}, HS{}, GS{};
  };
  void create(D3DGraphicsContext *ctx, const State &state,
              const std::vector<CD3DX12_ROOT_PARAMETER1> &rootParameters,
              const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElements,
              const Shaders &shaders, DXGI_FORMAT colorFormat,
              DXGI_FORMAT depthFormat);
  D3D_PRIMITIVE_TOPOLOGY d3dPrimitiveTopology;
  uint32_t d3dStencilRef = 0;
private:
  D3D12_PRIMITIVE_TOPOLOGY_TYPE
  getPrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY topology);
};
D3D_CAST(GraphicsPipeline);
}; // namespace ngfx
