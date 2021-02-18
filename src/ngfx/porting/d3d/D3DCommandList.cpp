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
#include "ngfx/porting/d3d/D3DCommandList.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
using namespace ngfx;

void D3DCommandList::create(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator, 
		D3D12_COMMAND_LIST_TYPE type) {
	HRESULT hResult;
	this->commandAllocator = commandAllocator;
	V(device->CreateCommandList(0, type, commandAllocator, nullptr, IID_PPV_ARGS(&v)));
	V(v->Close());
}

void D3DCommandList::begin() {
	HRESULT hResult;
	V(v->Reset(commandAllocator, nullptr));
}
void D3DCommandList::end() {
	HRESULT hResult;
	V(v->Close());
}