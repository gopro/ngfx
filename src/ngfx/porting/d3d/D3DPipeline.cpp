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
#include "ngfx/porting/d3d/D3DPipeline.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
using namespace ngfx;

void D3DPipeline::create(D3DGraphicsContext *ctx) { this->ctx = ctx; }

void D3DPipeline::createRootSignature(
    const std::vector<CD3DX12_ROOT_PARAMETER1> &rootParameters,
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags) {
  HRESULT hResult;
  auto d3dDevice = ctx->d3dDevice.v;
  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
  rootSignatureDesc.Init_1_1(UINT(rootParameters.size()), rootParameters.data(),
                             0, nullptr, rootSignatureFlags);

  ComPtr<ID3DBlob> error, signature;
  D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
  featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
  if (FAILED(d3dDevice->CheckFeatureSupport(
          D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
  }
  V(D3DX12SerializeVersionedRootSignature(
      &rootSignatureDesc, featureData.HighestVersion, &signature, &error));
  V(d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(),
                                   signature->GetBufferSize(),
                                   IID_PPV_ARGS(&d3dRootSignature)));
}
