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
#include "ngfx/compute/ComputePipeline.h"
#include "ngfx/graphics/Buffer.h"
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/Device.h"
#include "ngfx/graphics/GraphicsPipeline.h"
#include "ngfx/graphics/Texture.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace ngfx {
class Graphics {
public:
  static Graphics *create(GraphicsContext *ctx);
  virtual ~Graphics() {}

  virtual void beginComputePass(CommandBuffer *commandBuffer) = 0;
  virtual void endComputePass(CommandBuffer *commandBuffer) = 0;
  virtual void beginRenderPass(CommandBuffer *commandBuffer,
                               RenderPass *renderPass, Framebuffer *framebuffer,
                               glm::vec4 clearColor = glm::vec4(0.0f),
                               float clearDepth = 1.0f,
                               uint32_t clearStencil = 0) = 0;
  virtual void endRenderPass(CommandBuffer *commandBuffer) = 0;

  virtual void bindVertexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                uint32_t location, uint32_t stride) = 0;
  virtual void
  bindIndexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                  IndexFormat indexFormat = INDEXFORMAT_UINT32) = 0;
  virtual void bindUniformBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                 uint32_t binding,
                                 ShaderStageFlags shaderStageFlags) = 0;
  virtual void bindStorageBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                 uint32_t binding,
                                 ShaderStageFlags shaderStageFlags) = 0;
  virtual void bindComputePipeline(CommandBuffer *cmdBuffer,
                                   ComputePipeline *computePipeline) = 0;
  virtual void bindGraphicsPipeline(CommandBuffer *cmdBuffer,
                                    GraphicsPipeline *graphicsPipeline) = 0;
  virtual void bindTexture(CommandBuffer *commandBuffer, Texture *texture,
                           uint32_t set) = 0;

  // TODO: copyBuffer: ToBuffer, copyBuffer: ToTexture, copyTexture: ToBuffer,
  // blit

  virtual void draw(CommandBuffer *cmdBuffer, uint32_t vertexCount,
                    uint32_t instanceCount = 1, uint32_t firstVertex = 0,
                    uint32_t firstInstance = 0) = 0;
  virtual void drawIndexed(CommandBuffer *cmdBuffer, uint32_t indexCount,
                           uint32_t instanceCount = 1, uint32_t firstIndex = 0,
                           int32_t vertexOffset = 0,
                           uint32_t firstInstance = 0) = 0;

  virtual void dispatch(CommandBuffer *cmdBuffer, uint32_t groupCountX,
                        uint32_t groupCountY, uint32_t groupCountZ,
                        uint32_t threadsPerGroupX, uint32_t threadsPerGroupY,
                        uint32_t threadsPerGroupZ) = 0;

  virtual void setViewport(CommandBuffer *cmdBuffer, Rect2D rect) = 0;
  virtual void setScissor(CommandBuffer *cmdBuffer, Rect2D rect) = 0;

  virtual void waitIdle(CommandBuffer *cmdBuffer) = 0;

  Rect2D scissorRect;
  Rect2D viewport;
  Pipeline *currentPipeline = nullptr;
  RenderPass *currentRenderPass = nullptr;
  Framebuffer *currentFramebuffer = nullptr;

protected:
  GraphicsContext *ctx;
};
} // namespace ngfx
