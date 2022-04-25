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
#include "ngfx/porting/d3d/D3DSwapchain.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/core/StringUtil.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
#include "ngfx/core/Timer.h"
using namespace ngfx;
#define DEFAULT_SURFACE_FORMAT PIXELFORMAT_RGBA8_UNORM

void D3DSwapchain::create(D3DGraphicsContext *ctx, D3DSurface *surface) {
  HRESULT hResult;
  this->ctx = ctx;
  auto d3dFactory = ctx->d3dFactory.Get();
  auto d3dCommandQueue = ctx->d3dCommandQueue.v.Get();
  numImages = PREFERRED_NUM_SWAPCHAIN_IMAGES;
  this->w = surface->w; this->h = surface->h;
  this->format = DEFAULT_SURFACE_FORMAT;
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {w,
                                         h,
                                         DXGI_FORMAT(format),
                                         FALSE,
                                         {1, 0},
                                         DXGI_USAGE_RENDER_TARGET_OUTPUT,
                                         numImages,
                                         DXGI_SCALING_STRETCH,
                                         DXGI_SWAP_EFFECT_FLIP_DISCARD,
                                         DXGI_ALPHA_MODE_UNSPECIFIED,
                                         0};
  ComPtr<IDXGISwapChain1> swapchain;
  V(d3dFactory->CreateSwapChainForHwnd(d3dCommandQueue, surface->v,
                                       &swapChainDesc, nullptr, nullptr,
                                       &swapchain));
  V(swapchain.As(&v));
  getSwapchainRenderTargets();
  createSwapchainRenderTargetViews(w, h);
}

void D3DSwapchain::getSwapchainRenderTargets() {
  HRESULT hResult;
  renderTargets.resize(numImages);
  for (UINT j = 0; j < numImages; j++) {
    V(v->GetBuffer(j, IID_PPV_ARGS(&renderTargets[j])));
  }
}

void D3DSwapchain::createSwapchainRenderTargetViews(uint32_t w, uint32_t h) {
  HRESULT hResult;
  auto d3dDevice = ctx->d3dDevice.v.Get();
  auto rtvDescriptorHeap = &ctx->d3dRtvDescriptorHeap;
  renderTargetDescriptors.resize(numImages);
  for (UINT n = 0; n < numImages; n++) {
    V(v->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
    D3D_TRACE(d3dDevice->CreateRenderTargetView(
        renderTargets[n].Get(), nullptr, rtvDescriptorHeap->handle.cpuHandle));
    renderTargetDescriptors[n] = rtvDescriptorHeap->handle;
    ++rtvDescriptorHeap->handle;
  }
}

void D3DSwapchain::acquireNextImage() {
  ctx->currentImageIndex = v->GetCurrentBackBufferIndex();
  auto waitFence = ctx->frameFences[ctx->currentImageIndex];
  waitFence->wait();
  waitFence->reset();
}

void D3DSwapchain::present() {
  HRESULT hResult;
  V(v->Present(1, 0));
  ctx->currentImageIndex = -1;
}

void D3DSwapchain::setName(const std::string& name) {
    Swapchain::setName(name);
    for (uint32_t j = 0; j < numImages; j++) {
        auto& rt = renderTargets[j];
        std::wstring wname = StringUtil::toWString(name).c_str() + std::to_wstring(j);
        rt->SetName(wname.c_str());
    }
}
