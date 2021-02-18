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
#include <d3d12.h>

namespace ngfx {
struct D3DSamplerDesc : public D3D12_SAMPLER_DESC {
  D3DSamplerDesc() {
    Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    MipLODBias = 0;
    MaxAnisotropy = 0;
    ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    MinLOD = 0.0f;
    MaxLOD = D3D12_FLOAT32_MAX;
  }
};
} // namespace ngfx