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
#include "ngfx/graphics/RenderPass.h"
#include "ngfx/porting/d3d/D3DUtil.h"

namespace ngfx {
class D3DGraphicsContext;
class D3DRenderPass : public RenderPass {
public:
  void create(D3DGraphicsContext *ctx,
              D3D12_RESOURCE_STATES initialResourceState,
              D3D12_RESOURCE_STATES finalResourceState);
  virtual ~D3DRenderPass() {}
  D3D12_RESOURCE_STATES initialResourceState, finalResourceState;

private:
  D3DGraphicsContext *ctx;
  Framebuffer *currentFramebuffer = nullptr;
};
D3D_CAST(RenderPass);
} // namespace ngfx