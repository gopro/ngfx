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
#include "ngfx/graphics/Graphics.h"
#include "ngfx/porting/vulkan/VKUtil.h"
#include "ngfx/core/DebugUtil.h"

namespace ngfx {
class VKGraphics : public Graphics {
public:
  void create() {}
  virtual ~VKGraphics() {}
  void beginComputePass(CommandBuffer *commandBuffer) override {}
  void endComputePass(CommandBuffer *commandBuffer) override {}
  void beginRenderPass(CommandBuffer *commandBuffer, RenderPass *renderPass,
                       Framebuffer *framebuffer,
                       glm::vec4 clearColor = glm::vec4(0.0f),
                       float clearDepth = 1.0f,
                       uint32_t clearStencil = 0) override;
  void endRenderPass(CommandBuffer *commandBuffer) override;
  void beginProfile(CommandBuffer *commandBuffer) override;
  uint64_t endProfile(CommandBuffer *commandBuffer) override;
  void bindVertexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                        uint32_t location, uint32_t stride) override;
  void bindIndexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                       IndexFormat indexFormat) override;
  void bindUniformBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                         uint32_t binding,
                         ShaderStageFlags shaderStageFlags) override;
  void bindStorageBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                         uint32_t binding,
                         ShaderStageFlags shaderStageFlags,
                         bool readonly) override;
  void bindComputePipeline(CommandBuffer *cmdBuffer,
                           ComputePipeline *computePipeline) override;
  void bindGraphicsPipeline(CommandBuffer *cmdBuffer,
                            GraphicsPipeline *graphicsPipeline) override;
  void bindSampler(CommandBuffer* cmdBuffer, Sampler* sampler,
      uint32_t set) override {
      NGFX_TODO();
  }
  void bindTexture(CommandBuffer *commandBuffer, Texture *texture,
                   uint32_t set) override;
  void bindTextureAsImage(CommandBuffer* commandBuffer, Texture* texture,
      uint32_t set) override {
      NGFX_TODO();
  }
  void dispatch(CommandBuffer *cmdBuffer, uint32_t groupCountX,
                uint32_t groupCountY, uint32_t groupCountZ,
                int32_t threadsPerGroupX, int32_t threadsPerGroupY,
                int32_t threadsPerGroupZ) override;
  void draw(CommandBuffer *cmdBuffer, uint32_t vertexCount,
            uint32_t instanceCount = 1, uint32_t firstVertex = 0,
            uint32_t firstInstance = 0) override;
  void drawIndexed(CommandBuffer *cmdBuffer, uint32_t indexCount,
                   uint32_t instanceCount = 1, uint32_t firstIndex = 0,
                   int32_t vertexOffset = 0,
                   uint32_t firstInstance = 0) override;
  void setViewport(CommandBuffer *cmdBuffer, Rect2D rect) override;
  void setScissor(CommandBuffer *cmdBuffer, Rect2D rect) override;
  void waitIdle(CommandBuffer *cmdBuffer) override;
};
VK_CAST(Graphics);
} // namespace ngfx
