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
#include "ngfx/porting/d3d/D3DFence.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
using namespace ngfx;

void D3DFence::create(ID3D12Device *device, Value initialValue) {
  HRESULT hResult;
  V(device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&v)));
  fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void D3DFence::wait() {
  HRESULT hResult;
  if (v->GetCompletedValue() == SIGNALED)
    return;
  V(v->SetEventOnCompletion(SIGNALED, fenceEvent));
  D3D_TRACE(WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE));
}

void D3DFence::reset() {
  HRESULT hResult;
  V(v->Signal(UNSIGNALED));
}