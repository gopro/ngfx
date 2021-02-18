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
#include "ngfx/porting/d3d/D3DBuffer.h"
#include "ngfx/porting/d3d/D3DCommandList.h"
#include "ngfx/porting/d3d/D3DFramebuffer.h"
#include "ngfx/porting/d3d/D3DGraphicsPipeline.h"
#include <glm/glm.hpp>

namespace ngfx {
struct Region {
  glm::ivec3 p0, p1;
};
class D3DGraphicsContext;
class D3DTexture;
class D3DGraphics;
class D3DBlitOp {
public:
  D3DBlitOp(D3DGraphicsContext *ctx, D3DTexture *srcTexture, uint32_t srcLevel,
            D3DTexture *dstTexture, uint32_t dstLevel, Region srcRegion,
            Region dstRegion, uint32_t srcBaseLayer = 0,
            uint32_t srcLayerCount = 1, uint32_t dstBaseLayer = 0,
            uint32_t dstLayerCount = 1);
  void apply(D3DGraphicsContext *ctx, D3DCommandList *cmdList,
             D3DGraphics *graphics);
  void draw(D3DCommandList *cmdList, D3DGraphics *graphics);
  std::unique_ptr<D3DFramebuffer> outputFramebuffer;
  std::unique_ptr<D3DBuffer> bPos, bTexCoord, bUbo;
  D3DTexture *srcTexture, *dstTexture;
  uint32_t srcLevel, dstLevel;
  Region srcRegion, dstRegion;
  uint32_t srcBaseLayer, srcLayerCount, dstBaseLayer, dstLayerCount;

protected:
  virtual void createPipeline();
  struct UBOData {
    uint32_t lod, padding[3];
  };
  D3DGraphicsPipeline *graphicsPipeline;
  uint32_t numVerts;
  uint32_t B_POS, B_TEXCOORD, U_UBO, U_TEXTURE;
  D3DGraphicsContext *ctx;
};
}; // namespace ngfx