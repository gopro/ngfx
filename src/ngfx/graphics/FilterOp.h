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

/** \class FilterOp
 * 
 *  This is the base class for filter operations.
 *  A filter can output to a texture or to a framebuffer.
 *  TODO: support chained filters.
 */
 
namespace ngfx {
class FilterOp : public DrawOp {
public:
   /** Create a filter with external output texture */
   FilterOp(GraphicsContext* ctx, Graphics* graphics, Texture* outputTexture, bool enableDepthStencil);
  /** Create a filter operation, and allocate the output texture
   *  @param ctx The graphics context
   *  @param graphics The graphics interface
   *  @param dstWidth The destination width
   *  @param dstHeight The destination height
   *  @param enableDepthStencil Enable depth / stencil attachment
   */
  FilterOp(GraphicsContext *ctx, Graphics *graphics, uint32_t dstWidth,
           uint32_t dstHeight, bool enableDepthStencil = false);
  /** Destroy the filter operation */
  virtual ~FilterOp() {}
  /** Apply the filter
   *  @param ctx The graphics context
   *  @param commandBuffer The command buffer
   *  @param graphics The graphics interface
   */
  void apply(GraphicsContext *ctx, CommandBuffer *commandBuffer,
             Graphics *graphics);
  /** The output texture */
  Texture* outputTexture = nullptr;
  /** The depth / stencil attachment */
  std::unique_ptr<Texture> depthStencilTexture;
  /** The output framebuffer */
  std::unique_ptr<Framebuffer> outputFramebuffer;
private:
    void init(GraphicsContext* ctx, Graphics* graphics, uint32_t w, uint32_t h, bool enableDepthStencil);
    /** If output texture is allocated internally, stores the output texture */
    std::unique_ptr<Texture> outputTextureInternalPtr;
};
}; // namespace ngfx
