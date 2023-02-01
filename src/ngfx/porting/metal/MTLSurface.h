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
#include "ngfx/graphics/Surface.h"
#include "ngfx/porting/metal/MTLDepthStencilTexture.h"
#include "ngfx/porting/metal/MTLTexture.h"
#include "ngfx/porting/metal/MTLUtil.h"
#include <memory>
#include <MetalKit/MetalKit.h>

namespace ngfx {
class MTLSurface : public Surface {
public:
  virtual ~MTLSurface() {}
  inline CAMetalLayer *getMetalLayer() { return (CAMetalLayer *)view.layer; }
  NSView *view = nullptr;
  id<CAMetalDrawable> drawable;
  std::unique_ptr<MTLDepthStencilTexture> msaaDepthStencilTexture;
  std::unique_ptr<MTLDepthStencilTexture> depthStencilTexture;
  std::unique_ptr<MTLTexture> colorTexture;
  std::unique_ptr<MTLTexture> msaaColorTexture;
};
MTL_CAST(Surface);
} // namespace ngfx
