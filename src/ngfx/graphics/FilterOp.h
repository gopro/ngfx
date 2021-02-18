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
#include "ngfx/graphics/GraphicsContext.h"

namespace ngfx {
class FilterOp : public DrawOp {
public:
  FilterOp(GraphicsContext *ctx, Graphics *graphics, uint32_t dstWidth,
           uint32_t dstHeight);
  virtual ~FilterOp() {}
  void apply(GraphicsContext *ctx, CommandBuffer *commandBuffer,
             Graphics *graphics);
  std::unique_ptr<Texture> outputTexture;
  std::unique_ptr<Framebuffer> outputFramebuffer;
};
}; // namespace ngfx
