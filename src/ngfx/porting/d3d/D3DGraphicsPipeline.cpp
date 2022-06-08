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
#include "ngfx/porting/d3d/D3DGraphicsPipeline.h"
#include "ngfx/porting/d3d/D3DCommandList.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
#include "ngfx/porting/d3d/D3DShaderModule.h"
#include "ngfx/porting/d3d/D3DPipelineUtil.h"
#include <d3dx12.h>
using namespace ngfx;
using namespace std;

D3D12_PRIMITIVE_TOPOLOGY_TYPE
D3DGraphicsPipeline::getPrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY topology) {
  switch (topology) {
  case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
  case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    break;
  case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    break;
  case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
  case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    break;
  default:
    NGFX_ERR("topology: %d unsupported", topology);
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    break;
  }
}

void D3DGraphicsPipeline::create(
    D3DGraphicsContext *ctx, const State &state,
    const std::vector<CD3DX12_ROOT_PARAMETER1> &rootParameters,
    const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElements,
    const Shaders &shaders, DXGI_FORMAT colorFormat, DXGI_FORMAT depthStencilFormat) {
  D3DPipeline::create(ctx);
  HRESULT hResult;
  d3dPrimitiveTopology = state.primitiveTopology;
  auto d3dDevice = ctx->d3dDevice.v;
  createRootSignature(
      rootParameters,
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
  D3D12_RASTERIZER_DESC rasterizerState = {
      state.fillMode,
      state.cullMode,
      state.frontFaceCounterClockwise,
      0,
      0.0f,
      0.0f,
      state.depthTestEnable,
      FALSE,
      FALSE,
      0,
      D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};
  D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {
      state.blendEnable,         FALSE,
      state.blendSrcColorFactor, state.blendDstColorFactor,
      state.blendColorOp,        state.blendSrcAlphaFactor,
      state.blendDstAlphaFactor, state.blendAlphaOp,
      D3D12_LOGIC_OP_NOOP,       state.colorWriteMask};

  D3D12_BLEND_DESC blendDesc = {};
  blendDesc.AlphaToCoverageEnable = FALSE;
  blendDesc.IndependentBlendEnable = FALSE;
  blendDesc.RenderTarget[0] = renderTargetBlendDesc;

  CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
  depthStencilDesc.DepthEnable = state.depthTestEnable;
  depthStencilDesc.DepthWriteMask = state.depthWriteEnable ?
      D3D12_DEPTH_WRITE_MASK_ALL :
      D3D12_DEPTH_WRITE_MASK_ZERO;
  depthStencilDesc.DepthFunc = state.depthFunc;
  depthStencilDesc.StencilEnable = state.stencilEnable;
  depthStencilDesc.StencilReadMask = state.stencilReadMask;
  depthStencilDesc.StencilWriteMask = state.stencilWriteMask;
  depthStencilDesc.FrontFace.StencilDepthFailOp = state.frontStencilDepthFailOp;
  depthStencilDesc.FrontFace.StencilFailOp = state.frontStencilFailOp;
  depthStencilDesc.FrontFace.StencilFunc = state.frontStencilFunc;
  depthStencilDesc.FrontFace.StencilPassOp = state.frontStencilPassOp;
  depthStencilDesc.BackFace.StencilDepthFailOp = state.backStencilDepthFailOp;
  depthStencilDesc.BackFace.StencilFailOp = state.backStencilFailOp;
  depthStencilDesc.BackFace.StencilFunc = state.backStencilFunc;
  depthStencilDesc.BackFace.StencilPassOp = state.backStencilPassOp;
  d3dStencilRef = state.stencilRef;

  D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
  desc.InputLayout = {inputElements.data(), UINT(inputElements.size())};
  desc.pRootSignature = d3dRootSignature.Get();
  desc.VS = shaders.VS;
  desc.PS = shaders.PS;
  desc.GS = shaders.GS;
  desc.RasterizerState = rasterizerState;
  desc.BlendState = blendDesc;
  desc.DepthStencilState = depthStencilDesc;
  desc.DSVFormat = depthStencilFormat;
  desc.SampleMask = UINT_MAX;
  desc.PrimitiveTopologyType =
      getPrimitiveTopologyType(state.primitiveTopology);
  desc.NumRenderTargets = state.numColorAttachments;
  for (uint32_t j = 0; j < desc.NumRenderTargets; j++)
    desc.RTVFormats[j] = colorFormat;
  desc.SampleDesc.Count = state.numSamples;

  V(d3dDevice->CreateGraphicsPipelineState(&desc,
                                           IID_PPV_ARGS(&d3dPipelineState)));
}

GraphicsPipeline *
GraphicsPipeline::create(GraphicsContext *graphicsContext, const State &state,
                         VertexShaderModule *vs, FragmentShaderModule *fs,
                         PixelFormat colorFormat, PixelFormat depthStencilFormat,
                         std::vector<VertexInputAttributeDescription> vertexAttributes,
                         std::set<std::string> instanceAttributes) {
  D3DGraphicsPipeline *d3dGraphicsPipeline = new D3DGraphicsPipeline();

  auto getAlphaBlendFactor = [](BlendFactor blendFactor) -> BlendFactor {
    switch (blendFactor) {
    case BLEND_FACTOR_SRC_COLOR:
      return BLEND_FACTOR_SRC_ALPHA;
      break;
    case BLEND_FACTOR_DST_COLOR:
      return BLEND_FACTOR_DST_ALPHA;
      break;
    case BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
      return BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      break;
    case BLEND_FACTOR_ONE_MINUS_DST_COLOR:
      return BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
      break;
    default:
      return blendFactor;
      break;
    };
  };
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
    D3D12_COLOR_WRITE_ENABLE colorWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK;
    bool frontFaceCounterClockwise = false;
    float lineWidth = 1.0f;
    bool depthTestEnable = false, depthWriteEnable = false;
    D3DRenderPass *renderPass = nullptr;
    uint32_t numSamples = 1, numColorAttachments = 1;
  };
  D3DGraphicsPipeline::State d3dState = {
      D3D_PRIMITIVE_TOPOLOGY(state.primitiveTopology),
      D3D12_FILL_MODE(state.polygonMode),
      state.blendEnable,
      D3D12_BLEND(state.srcColorBlendFactor),
      D3D12_BLEND(state.dstColorBlendFactor),
      D3D12_BLEND_OP(state.colorBlendOp),
      D3D12_BLEND(getAlphaBlendFactor(state.srcAlphaBlendFactor)),
      D3D12_BLEND(getAlphaBlendFactor(state.dstAlphaBlendFactor)),
      D3D12_BLEND_OP(state.alphaBlendOp),
      state.colorWriteMask,
      D3D12_CULL_MODE(state.cullModeFlags),
      (state.frontFace == FRONT_FACE_COUNTER_CLOCKWISE),
      state.lineWidth,
      state.depthTestEnable,
      state.depthWriteEnable,
      D3D12_COMPARISON_FUNC(state.depthFunc),
      state.stencilEnable,
      state.stencilReadMask,
      state.stencilWriteMask,
      D3D12_STENCIL_OP(state.frontStencilFailOp),
      D3D12_STENCIL_OP(state.frontStencilDepthFailOp),
      D3D12_STENCIL_OP(state.frontStencilPassOp),
      D3D12_COMPARISON_FUNC(state.frontStencilFunc),
      D3D12_STENCIL_OP(state.backStencilFailOp),
      D3D12_STENCIL_OP(state.backStencilDepthFailOp),
      D3D12_STENCIL_OP(state.backStencilPassOp),
      D3D12_COMPARISON_FUNC(state.backStencilFunc),
      state.stencilRef,
      state.numSamples,
      state.numColorAttachments};
  auto &descriptorBindings = d3dGraphicsPipeline->descriptorBindings;

  std::vector<CD3DX12_ROOT_PARAMETER1> d3dRootParams;
  std::vector<std::unique_ptr<CD3DX12_DESCRIPTOR_RANGE1>> d3dDescriptorRanges;

  std::map<uint32_t, ShaderModule::DescriptorInfo> descriptors;
  uint32_t descriptorBindingsSize = 0;
  for (auto& descriptor : vs->descriptors) {
      descriptors[descriptor.set] = descriptor;
      descriptorBindingsSize = std::max(descriptorBindingsSize, descriptor.set + 1);
  }
  for (auto& descriptor : fs->descriptors) {
      descriptors[descriptor.set] = descriptor;
      descriptorBindingsSize = std::max(descriptorBindingsSize, descriptor.set + 1);
  }
  uint32_t numDescriptors = uint32_t(descriptors.size());
  descriptorBindings.resize(descriptorBindingsSize);
  D3DPipelineUtil::IsReadOnly isReadOnly = [&](const ShaderModule::DescriptorInfo &descriptorInfo) -> bool {
      if (descriptorInfo.type == DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
          auto bufferInfo = vs->findUniformBufferInfo(descriptorInfo.name);
          if (!bufferInfo)
              bufferInfo = fs->findUniformBufferInfo(descriptorInfo.name);
          return bufferInfo->readonly;
      }
      else if (descriptorInfo.type == DESCRIPTOR_TYPE_STORAGE_BUFFER) {
          auto bufferInfo = vs->findStorageBufferInfo(descriptorInfo.name);
          if (!bufferInfo)
              bufferInfo = fs->findStorageBufferInfo(descriptorInfo.name);
          return bufferInfo->readonly;
      }
      else
          return false;
  };
  D3DPipelineUtil::parseDescriptors(descriptors, descriptorBindings,
                                    d3dRootParams, d3dDescriptorRanges,
                                    D3DPipelineUtil::PIPELINE_TYPE_GRAPHICS, isReadOnly);

  std::vector<D3D12_INPUT_ELEMENT_DESC> d3dVertexInputAttributes(
      vs->attributes.size());
  auto &vertexAttributeBindings = d3dGraphicsPipeline->vertexAttributeBindings;
  vertexAttributeBindings.resize(vs->attributes.size());
  struct SemanticData {
    string name;
    uint32_t index;
  };
  std::vector<SemanticData> semanticData(vs->attributes.size());
  for (int j = 0; j < vs->attributes.size(); j++) {
    const auto &va = vs->attributes[j];
    uint32_t binding = va.location,  // TODO: va.count
        offset = vertexAttributes.empty() ? 0 : vertexAttributes[j].offset;
    D3D12_INPUT_CLASSIFICATION inputRate =
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    if (instanceAttributes.find(va.name) != instanceAttributes.end())
      inputRate = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    int32_t semanticIndexOffset =
        va.semantic.find_first_of("0123456789");
    string &semanticName = semanticData[j].name;
    uint32_t &semanticIndex = semanticData[j].index;
    if (semanticIndexOffset != string::npos) {
      semanticName = va.semantic.substr(0, semanticIndexOffset);
      string semanticIndexStr = va.semantic.substr(semanticIndexOffset);
      semanticIndex = atoi(semanticIndexStr.c_str());
    } else {
      semanticName = va.semantic;
      semanticIndex = 0;
    }
    d3dVertexInputAttributes[j] = {
        semanticName.c_str(),
        semanticIndex,
        DXGI_FORMAT(va.format),
        binding,
        offset,
        inputRate,
        (inputRate == D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA) ? UINT(0)
                                                                  : UINT(1)};
    vertexAttributeBindings[j] = j;
  }

  D3DGraphicsPipeline::Shaders shaders;
  shaders.VS = d3d(vs)->d3dShaderByteCode;
  shaders.PS = d3d(fs)->d3dShaderByteCode;
  d3dGraphicsPipeline->create(
      d3d(graphicsContext), d3dState, d3dRootParams, d3dVertexInputAttributes,
      shaders, DXGI_FORMAT(colorFormat), DXGI_FORMAT(depthStencilFormat));

  return d3dGraphicsPipeline;
}
