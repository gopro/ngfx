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

void D3DPipeline::create(D3DGraphicsContext* ctx) {
    this->ctx = ctx;
}

void D3DPipeline::createRootSignature(
    const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParameters,
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags) {
    HRESULT hResult;
    auto d3dDevice = ctx->d3dDevice.v;
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(UINT(rootParameters.size()), rootParameters.data(), 0, nullptr, rootSignatureFlags);

    ComPtr<ID3DBlob> error, signature;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    V(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
    V(d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&d3dRootSignature)));
}

void D3DPipelineUtil::parseDescriptors(
        std::map<uint32_t, ShaderModule::DescriptorInfo> &descriptors,
        std::vector<uint32_t> &descriptorBindings,
        std::vector<CD3DX12_ROOT_PARAMETER1> &d3dRootParams,
        std::vector<std::unique_ptr<CD3DX12_DESCRIPTOR_RANGE1>> &d3dDescriptorRanges,
        PipelineType pipelineType
    ) {
    int registerSpace = 0;
    for (const auto& it : descriptors) {
        auto& descriptor = it.second;
        descriptorBindings[registerSpace] = uint32_t(d3dRootParams.size());
        if (descriptor.type == DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            CD3DX12_ROOT_PARAMETER1 d3dSrvDescriptor, d3dSamplerDescriptor;
            auto d3dSrvDescriptorRange = std::make_unique<CD3DX12_DESCRIPTOR_RANGE1>();
            d3dSrvDescriptorRange->Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, registerSpace);
            d3dSrvDescriptor.InitAsDescriptorTable(1, d3dSrvDescriptorRange.get());
            d3dDescriptorRanges.emplace_back(std::move(d3dSrvDescriptorRange));
            d3dRootParams.emplace_back(std::move(d3dSrvDescriptor));

            auto d3dSamplerDescriptorRange = std::make_unique<CD3DX12_DESCRIPTOR_RANGE1>();
            d3dSamplerDescriptorRange->Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, registerSpace);
            d3dSamplerDescriptor.InitAsDescriptorTable(1, d3dSamplerDescriptorRange.get());
            d3dDescriptorRanges.emplace_back(std::move(d3dSamplerDescriptorRange));
            d3dRootParams.emplace_back(std::move(d3dSamplerDescriptor));
        }
        else if (descriptor.type == DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            CD3DX12_ROOT_PARAMETER1 d3dDescriptor;
            d3dDescriptor.InitAsConstantBufferView(0, registerSpace);
            d3dRootParams.emplace_back(std::move(d3dDescriptor));
        }
        else if (descriptor.type == DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            CD3DX12_ROOT_PARAMETER1 d3dDescriptor;
            //TODO encode access flags as read-only or read-write
            if (pipelineType == PIPELINE_TYPE_GRAPHICS) 
                d3dDescriptor.InitAsShaderResourceView(0, registerSpace);
            else
                d3dDescriptor.InitAsUnorderedAccessView(0, registerSpace);
            d3dRootParams.emplace_back(std::move(d3dDescriptor));
        }
        else {
            CD3DX12_ROOT_PARAMETER1 d3dDescriptor;
            d3dDescriptor.InitAsUnorderedAccessView(0, registerSpace);
            d3dRootParams.emplace_back(std::move(d3dDescriptor));
        }
        registerSpace++;
    }
}