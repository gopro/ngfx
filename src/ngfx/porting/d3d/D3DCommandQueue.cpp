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
#include "ngfx/porting/d3d/D3DCommandQueue.h"
#include "ngfx/porting/d3d/D3DCommandList.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
using namespace ngfx;

void D3DCommandQueue::create(D3DGraphicsContext *ctx) {
  HRESULT hResult;
  this->ctx = ctx;
  auto d3dDevice = ctx->d3dDevice.v.Get();
  D3D12_COMMAND_QUEUE_DESC queueDesc = {D3D12_COMMAND_LIST_TYPE_DIRECT, 0,
                                        D3D12_COMMAND_QUEUE_FLAG_NONE,
                                        D3D12_COMMAND_LIST_TYPE_DIRECT};
  V(d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&v)));
}
void D3DCommandQueue::present() { ctx->d3dSwapchain.present(); }
void D3DCommandQueue::submit(CommandBuffer *commandBuffer) {
  ID3D12CommandList *d3dCommandList = d3d(commandBuffer)->v.Get();
  ID3D12Fence *fence = nullptr;
  if (d3dCommandList == ctx->d3dCopyCommandList.v.Get()) {
  } else if (d3dCommandList == ctx->d3dComputeCommandList.v.Get()) {
  } else {
    fence = ctx->d3dWaitFences[ctx->currentImageIndex].v.Get();
  }
  submit(d3dCommandList, fence);
}
void D3DCommandQueue::submit(ID3D12CommandList *commandList,
                             ID3D12Fence *fence) {
  HRESULT hResult;
  D3D_TRACE(v->ExecuteCommandLists(1, &commandList));
  if (fence)
    V(v->Signal(fence, D3DFence::SIGNALED));
}
void D3DCommandQueue::waitIdle() {
  HRESULT hResult;
  D3DFence fence;
  fence.create(ctx->d3dDevice.v.Get());
  // Schedule a Signal command in the queue.
  V(v->Signal(fence.v.Get(), D3DFence::SIGNALED));
  fence.wait();
}