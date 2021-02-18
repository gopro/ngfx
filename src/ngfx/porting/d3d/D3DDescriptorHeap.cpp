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
#include "ngfx/porting/d3d/D3DDescriptorHeap.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
using namespace ngfx;

void D3DDescriptorHeap::create(ID3D12Device *d3dDevice,
                               D3D12_DESCRIPTOR_HEAP_TYPE type,
                               UINT maxDescriptors,
                               D3D12_DESCRIPTOR_HEAP_FLAGS flags) {
  HRESULT hResult;
  this->type = type;
  this->maxDescriptors = maxDescriptors;
  D3D12_DESCRIPTOR_HEAP_DESC desc = {type, maxDescriptors, flags, 0};
  V(d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&v)));
  D3D_TRACE(handle.cpuHandle = v->GetCPUDescriptorHandleForHeapStart());
  D3D_TRACE(handle.descriptorSize =
                d3dDevice->GetDescriptorHandleIncrementSize(type));
  D3D_TRACE(handle.gpuHandle = v->GetGPUDescriptorHandleForHeapStart());
}