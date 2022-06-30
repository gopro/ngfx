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
#include "ngfx/graphics/DrawOp.h"
#include "ngfx/graphics/Graphics.h"

#include <memory>

namespace ngfx {
class DrawColorOp : public DrawOp {
public:
  template <typename T = glm::vec2>
  DrawColorOp(GraphicsContext* ctx, std::vector<T> pos, glm::vec4 color, 
      OnGetPipelineState onGetPipelineState = nullptr,
      std::vector<glm::i32> index = {})
      : DrawOp(ctx, onGetPipelineState) {
      bPos.reset(createVertexBuffer<T>(ctx, pos));
      if (!index.empty()) {
          bIndex.reset(createIndexBuffer<glm::i32> (ctx, index));
          numIndices = index.size();
      }
      bUbo.reset(createUniformBuffer(ctx, &color, sizeof(color)));
      numVerts = uint32_t(pos.size());
      createPipeline();
      graphicsPipeline->getBindings({ &U_UBO }, { &B_POS });
  }
  virtual ~DrawColorOp() {}
  void draw(CommandBuffer *commandBuffer, Graphics *graphics) override;
  std::unique_ptr<Buffer> bPos;
  std::unique_ptr<Buffer> bIndex;
  std::unique_ptr<Buffer> bUbo;

protected:
  virtual void createPipeline();
  GraphicsPipeline::State getPipelineState();
  GraphicsPipeline *graphicsPipeline;
  uint32_t B_POS, U_UBO;
  uint32_t numVerts, numIndices = 0;
};
} // namespace ngfx
