/*
 * Copyright 2021 GoPro Inc.
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

#include "ngfx/porting/d3d/D3DSampler.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
using namespace ngfx;
using namespace std;

void D3DSampler::create(D3DGraphicsContext* ctx, const D3DSamplerDesc &samplerDesc) {
	desc = samplerDesc;
	D3D_TRACE(ctx->d3dDevice.v->CreateSampler(&samplerDesc, ctx->d3dSamplerDescriptorHeap.handle.cpuHandle));
	handle = ctx->d3dSamplerDescriptorHeap.handle;
	++ctx->d3dSamplerDescriptorHeap.handle;
}

Sampler* Sampler::create(GraphicsContext *ctx, const SamplerDesc &samplerDesc) {
	D3DSamplerDesc d3dSamplerDesc;
	static const vector<D3D12_FILTER> filterMap = {
		 D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR,
		D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT,
		D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR
	};
	uint32_t filterIndex = samplerDesc.minFilter << 2 | samplerDesc.magFilter << 1 | samplerDesc.mipFilter;
	d3dSamplerDesc.Filter = filterMap.at(filterIndex);
    d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE(samplerDesc.addressModeU);
    d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE(samplerDesc.addressModeV);
    d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE(samplerDesc.addressModeW);
	D3DSampler* d3dSampler = new D3DSampler();
	d3dSampler->create(d3d(ctx), d3dSamplerDesc);
	return d3dSampler;
}