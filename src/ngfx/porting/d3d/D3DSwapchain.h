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
#include "ngfx/graphics/Swapchain.h"
#include "ngfx/porting/d3d/D3DDescriptorHeap.h"
#include "ngfx/porting/d3d/D3DDevice.h"
#include "ngfx/porting/d3d/D3DSurface.h"

namespace ngfx {
class D3DGraphicsContext;
class D3DSwapchain : public Swapchain {
public:
  void create(D3DGraphicsContext *ctx, D3DSurface *surface);
  virtual ~D3DSwapchain() {}
  void acquireNextImage() override;
  void present();
  void setName(const std::string& name) override;
  ComPtr<IDXGISwapChain3> v;
  std::vector<ComPtr<ID3D12Resource>> renderTargets;
  std::vector<D3DDescriptorHandle> renderTargetDescriptors;

private:
  void getSwapchainRenderTargets();
  void createSwapchainRenderTargetViews(uint32_t w, uint32_t h);
  D3DGraphicsContext *ctx;
};
} // namespace ngfx