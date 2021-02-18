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
  DrawColorOp(GraphicsContext *ctx, const std::vector<glm::vec2> &pos,
              const glm::vec4 &color);
  virtual ~DrawColorOp() {}
  void draw(CommandBuffer *commandBuffer, Graphics *graphics) override;
  std::unique_ptr<Buffer> bPos;
  std::unique_ptr<Buffer> bUbo;

protected:
  virtual void createPipeline();
  GraphicsPipeline *graphicsPipeline;
  uint32_t B_POS, U_UBO;
  uint32_t numVerts;
};
} // namespace ngfx
