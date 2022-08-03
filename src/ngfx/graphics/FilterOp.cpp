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
#include "ngfx/graphics/FilterOp.h"
#include "ngfx/graphics/FormatUtil.h"
using namespace ngfx;
using namespace std;

FilterOp::FilterOp(GraphicsContext *ctx, Graphics *graphics, uint32_t dstWidth,
                   uint32_t dstHeight, bool enableDepthStencil)
    : DrawOp(ctx) {
  uint32_t w = dstWidth, h = dstHeight, size = w * h * 4;
  SamplerDesc samplerDesc = {
            FILTER_LINEAR, FILTER_LINEAR, FILTER_LINEAR,
            CLAMP_TO_EDGE, CLAMP_TO_EDGE, CLAMP_TO_EDGE
  };
  outputTexture.reset(Texture::create(
      ctx, graphics, nullptr, PIXELFORMAT_RGBA8_UNORM, size, w, h, 1, 1,
      ImageUsageFlags(IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_DST_BIT |
                      IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
      TEXTURE_TYPE_2D, false, 1, &samplerDesc));
  if (enableDepthStencil) {
      uint32_t depthStencilSize = w * h * FormatUtil::getBytesPerPixel(ctx->depthStencilFormat);
      depthStencilTexture.reset(Texture::create(
          ctx, graphics, nullptr, ctx->depthStencilFormat, depthStencilSize,
          w, h, 1, 1, ImageUsageFlags(IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT),
          TEXTURE_TYPE_2D, false, 1, nullptr
      ));
  }
  vector<Framebuffer::Attachment> attachments = { {  outputTexture.get() } };
  if (enableDepthStencil)
      attachments.push_back({ { depthStencilTexture.get() } });
  outputFramebuffer.reset(Framebuffer::create(ctx->device,
                                              ctx->defaultOffscreenRenderPass,
                                              attachments, w, h));
}

void FilterOp::apply(GraphicsContext *ctx, CommandBuffer *commandBuffer,
                     Graphics *graphics) {
  outputTexture->changeLayout(commandBuffer,
                              IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  ctx->beginOffscreenRenderPass(commandBuffer, graphics,
                                outputFramebuffer.get());
  draw(commandBuffer, graphics);
  ctx->endOffscreenRenderPass(commandBuffer, graphics);
  outputTexture->changeLayout(commandBuffer,
                              IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
