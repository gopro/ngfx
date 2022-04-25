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
#include "ngfx/core/DebugUtil.h"
#include "ngfx/graphics/GraphicsContext.h"
#include "ngfx/porting/d3d/D3DCommandList.h"
#include "ngfx/porting/d3d/D3DCommandQueue.h"
#include "ngfx/porting/d3d/D3DComputePass.h"
#include "ngfx/porting/d3d/D3DDescriptorHeap.h"
#include "ngfx/porting/d3d/D3DDevice.h"
#include "ngfx/porting/d3d/D3DFence.h"
#include "ngfx/porting/d3d/D3DFramebuffer.h"
#include "ngfx/porting/d3d/D3DPipelineCache.h"
#include "ngfx/porting/d3d/D3DRenderPass.h"
#include "ngfx/porting/d3d/D3DSwapchain.h"
#include "ngfx/porting/d3d/D3DTexture.h"
#include "ngfx/porting/d3d/D3DUtil.h"
#include "ngfx/porting/d3d/D3DQueryHeap.h"
#include "ngfx/porting/d3d/D3DReadbackBuffer.h"
#include <memory>

namespace ngfx {
class D3DGraphicsContext : public GraphicsContext {
public:
  void create(const char *appName, bool enableDepthStencil, bool debug);
  virtual ~D3DGraphicsContext() {}
  void setSurface(Surface *surface) override;
  CommandBuffer *drawCommandBuffer(int32_t index = -1) override;
  CommandBuffer *copyCommandBuffer() override;
  CommandBuffer *computeCommandBuffer() override;
  struct D3DRenderPassData {
    RenderPassConfig config;
    D3DRenderPass d3dRenderPass;
  };
  RenderPass *getRenderPass(RenderPassConfig config) override;
  std::vector<std::unique_ptr<D3DRenderPassData>> d3dRenderPassCache;
  ComPtr<IDXGIFactory4> d3dFactory;
  D3DDevice d3dDevice;
  D3DCommandQueue d3dCommandQueue;
  D3DDescriptorHeap d3dRtvDescriptorHeap, d3dCbvSrvUavDescriptorHeap,
      d3dSamplerDescriptorHeap, d3dDsvDescriptorHeap;
  D3DPipelineCache d3dPipelineCache;
  std::unique_ptr<D3DSwapchain> d3dSwapchain;
  std::vector<D3DCommandList> d3dDrawCommandLists;
  D3DCommandList d3dOffscreenDrawCommandList;
  D3DCommandList d3dCopyCommandList;
  D3DCommandList d3dComputeCommandList;
  D3DRenderPass *d3dDefaultRenderPass = nullptr,
                *d3dDefaultOffscreenRenderPass = nullptr;
  std::vector<D3DFramebuffer> d3dSwapchainFramebuffers;
  std::vector<D3DFence> d3dDrawFences;
  D3DFence d3dCopyFence;
  D3DFence d3dComputeFence;
  D3DComputePass d3dComputePass;
  std::unique_ptr<D3DTexture> d3dDepthStencilView;
  D3DQueryHeap d3dQueryTimestampHeap;
  D3DReadbackBuffer d3dTimestampResultBuffer;
  bool offscreen = true;
  uint32_t numSamples = 1;

private:
  void createBindings();
  void createDescriptorHeaps();
  void createRenderPass(const RenderPassConfig &config,
                        D3DRenderPass &renderPass);
  void createFences(ID3D12Device *device);
  void createSwapchainFramebuffers(int w, int h);
};
D3D_CAST(GraphicsContext);
} // namespace ngfx
