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
#include "ngfx/porting/d3d/D3DGraphicsContext.h"

namespace ngfx {
class D3DPipeline {
public:
  void create(D3DGraphicsContext *ctx);
  virtual ~D3DPipeline() {}
  ComPtr<ID3D12PipelineState> d3dPipelineState;
  ComPtr<ID3D12RootSignature> d3dRootSignature;

protected:
  void createRootSignature(
      const std::vector<CD3DX12_ROOT_PARAMETER1> &rootParameters,
      D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
          D3D12_ROOT_SIGNATURE_FLAG_NONE);
  D3DGraphicsContext *ctx;
};

struct D3DPipelineUtil {
  enum PipelineType { PIPELINE_TYPE_COMPUTE, PIPELINE_TYPE_GRAPHICS };
  static void
  parseDescriptors(std::map<uint32_t, ShaderModule::DescriptorInfo> &uniforms,
                   std::vector<uint32_t> &uniformBindings,
                   std::vector<CD3DX12_ROOT_PARAMETER1> &d3dRootParams,
                   std::vector<std::unique_ptr<CD3DX12_DESCRIPTOR_RANGE1>>
                       &d3dDescriptorRanges,
                   PipelineType pipelineType);
};
}; // namespace ngfx