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
#include "ngfx/graphics/Buffer.h"
#include "ngfx/graphics/DrawOp.h"
#include <memory>

namespace ngfx {
class DrawTextureOp : public DrawOp {
public:
  DrawTextureOp(GraphicsContext *ctx, Texture *texture, OnGetPipelineState onGetPipelineState = nullptr)
      : DrawTextureOp(ctx, texture,
                      {glm::vec2(-1, 1), glm::vec2(-1, -1), glm::vec2(1, 1),
                       glm::vec2(1, -1)},
                      {glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 0),
                       glm::vec2(1, 1)},
                      onGetPipelineState) {}
  DrawTextureOp(GraphicsContext *ctx, Texture *texture,
                const std::vector<glm::vec2> &pos,
                const std::vector<glm::vec2> &texCoord,
                OnGetPipelineState onGetPipelineState);
  virtual ~DrawTextureOp() {}
  void draw(CommandBuffer *commandBuffer, Graphics *graphics) override;
  std::unique_ptr<Buffer> bPos, bTexCoord;
  Texture *texture;

protected:
  GraphicsPipeline::State getPipelineState();
  virtual void getPipeline();
  GraphicsPipeline *graphicsPipeline = nullptr;
  uint32_t numVerts;
  uint32_t B_POS, B_TEXCOORD, U_TEXTURE;
};
} // namespace ngfx
