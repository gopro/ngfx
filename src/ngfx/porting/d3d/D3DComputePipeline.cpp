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
#include "ngfx/porting/d3d/D3DComputePipeline.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DShaderModule.h"
#include "ngfx/porting/d3d/D3DPipelineUtil.h"
using namespace ngfx;

void D3DComputePipeline::create(
    D3DGraphicsContext *ctx,
    const std::vector<CD3DX12_ROOT_PARAMETER1> &rootParameters,
    D3D12_SHADER_BYTECODE shaderByteCode) {
  D3DPipeline::create(ctx);
  HRESULT hResult;
  auto d3dDevice = ctx->d3dDevice.v;
  createRootSignature(rootParameters);
  D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
  desc.pRootSignature = d3dRootSignature.Get();
  desc.CS.pShaderBytecode = shaderByteCode.pShaderBytecode;
  desc.CS.BytecodeLength = shaderByteCode.BytecodeLength;
  V(d3dDevice->CreateComputePipelineState(&desc,
                                          IID_PPV_ARGS(&d3dPipelineState)));
}

ComputePipeline *ComputePipeline::create(GraphicsContext *graphicsContext,
                                         ComputeShaderModule *cs) {
  D3DComputePipeline *d3dComputePipeline = new D3DComputePipeline();

  std::vector<CD3DX12_ROOT_PARAMETER1> d3dRootParams;
  std::vector<D3D12_STATIC_SAMPLER_DESC> d3dSamplers;
  std::vector<std::unique_ptr<CD3DX12_DESCRIPTOR_RANGE1>> d3dDescriptorRanges;
  auto &descriptorBindings = d3dComputePipeline->descriptorBindings;
  uint32_t numDescriptors = uint32_t(cs->descriptors.size());
  std::map<uint32_t, ShaderModule::DescriptorInfo> descriptors;
  uint32_t descriptorBindingsSize = 0;
  for (auto& csDescriptor : cs->descriptors) {
      descriptors[csDescriptor.set] = csDescriptor;
      descriptorBindingsSize = std::max(descriptorBindingsSize, csDescriptor.set + 1);
  }
  descriptorBindings.resize(descriptorBindingsSize);
  D3DPipelineUtil::IsReadOnly isReadOnly = [&](const ShaderModule::DescriptorInfo& descriptorInfo) -> bool {
      if (descriptorInfo.type == DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
          auto bufferInfo = cs->findUniformBufferInfo(descriptorInfo.name);
          return bufferInfo->readonly;
      }
      else if (descriptorInfo.type == DESCRIPTOR_TYPE_STORAGE_BUFFER) {
          auto bufferInfo = cs->findStorageBufferInfo(descriptorInfo.name);
          return bufferInfo->readonly;
      }
      else
        return false;
  };
  D3DPipelineUtil::parseDescriptors(descriptors, descriptorBindings,
                                    d3dRootParams, d3dDescriptorRanges,
                                    D3DPipelineUtil::PIPELINE_TYPE_COMPUTE, isReadOnly);

  d3dComputePipeline->create(d3d(graphicsContext), d3dRootParams,
                             d3d(cs)->d3dShaderByteCode);
  return d3dComputePipeline;
}