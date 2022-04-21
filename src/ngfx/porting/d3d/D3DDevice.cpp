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
#include "ngfx/porting/d3d/D3DDevice.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DFence.h"
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
#include "ngfx/core/StringUtil.h"
using namespace ngfx;

void D3DDevice::create(D3DGraphicsContext *ctx) {
  this->ctx = ctx;
  HRESULT hResult;
  auto factory = ctx->d3dFactory.Get();
  hardwareAdapter = nullptr;
  for (UINT adapterIndex = 0;
       DXGI_ERROR_NOT_FOUND !=
       factory->EnumAdapters1(adapterIndex, &hardwareAdapter);
       adapterIndex++) {
    DXGI_ADAPTER_DESC1 desc;
    hardwareAdapter->GetDesc1(&desc);
    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
      continue;

    const char* gpu_filter_env = getenv("GPU_FILTER");
    std::wstring gpu_filter = gpu_filter_env ? StringUtil::toWString(gpu_filter_env) : L"";
    //Skip GPUs not matching user filter
    if (wcsstr(desc.Description, gpu_filter.c_str()) == nullptr)
        continue;

    // Check to see if the adapter supports Direct3D 12
    if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(),
                                    D3D_FEATURE_LEVEL_11_0,
                                    _uuidof(ID3D12Device), nullptr)))
      break;
  }
  V(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                      IID_PPV_ARGS(&v)));
}

void D3DDevice::waitIdle() {
    ctx->d3dCommandQueue.waitIdle();
}