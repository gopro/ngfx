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
#include "ngfx/graphics/Sampler.h"
#include "ngfx/graphics/Texture.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace ngfx {

/** \class Graphics
 *
 *  This class defines the interface for a graphics commands module.
 *  These commands don't get executed immediately, they get recorded to a
 *  command buffer.
 *  Each backend is responsible for implementing this interface via a subclass.
 */

class Graphics {
public:
  /** Create the graphics module
   *  @param GraphicsContext The graphics context
   */
  static Graphics *create(GraphicsContext *ctx);
  /** Destroy the graphics module */
  virtual ~Graphics() {}

  /** Begin a compute pass */
  virtual void beginComputePass(CommandBuffer *commandBuffer) = 0;
  /** End a compute pass */
  virtual void endComputePass(CommandBuffer *commandBuffer) = 0;
  /** Begin a render pass
  *   @param commandBuffer The graphics command buffer
  *   @param renderPass    The render pass object
  *   @param framebuffer   The target framebuffer
  *   @param clearColor    The clear color
  *   @param clearDepth    The depth buffer clear value
  *   @param clearStencil  The stencil buffer clear value
  */
  virtual void beginRenderPass(CommandBuffer *commandBuffer,
                               RenderPass *renderPass, Framebuffer *framebuffer,
                               glm::vec4 clearColor = glm::vec4(0.0f),
                               float clearDepth = 1.0f,
                               uint32_t clearStencil = 0) = 0;
  /** End the render pass
  *   @param commandBuffer The graphics command buffer
  */
  virtual void endRenderPass(CommandBuffer *commandBuffer) = 0;
  /** Begin GPU profiling.
      Use beginProfile/endProfile to profile a group of commands in the command buffer.
  *   @param commandBuffer The command buffer
  */
  virtual void beginProfile(CommandBuffer *commandBuffer) = 0;
  /** End GPU profiling
  *   @param commandBuffer The command buffer
  *   @return The profiling result
  */
  virtual uint64_t endProfile(CommandBuffer *commandBuffer) = 0;
  /** Bind a buffer as a per-vertex input to the vertex shader module
   *  @param commandBuffer The command buffer
   *  @param buffer The input buffer
   *  @param location The target attribute location
   *  @param stride The size of each element (bytes)
   */
  virtual void bindVertexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                uint32_t location, uint32_t stride) = 0;
  /** Bind a buffer of vertex indices, for indexed drawing
   *  @param commandBuffer The command buffer
   *  @param buffer The input buffer
   *  @param indexFormat the format of the indices
   */
  virtual void
  bindIndexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                  IndexFormat indexFormat = INDEXFORMAT_UINT32) = 0;
  /** Bind a buffer as uniform input to shader module(s).
  *   A uniform buffer is stored in the GPU's high-speed cache memory, with a size limitation,
      and the GPU has read-only access.
  *   @param commandBuffer The command buffer
  *   @param buffer The input buffer
  *   @param binding The target binding
  *   @param shaderStageFlags The target shader module(s)
  */
  virtual void bindUniformBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                 uint32_t binding,
                                 ShaderStageFlags shaderStageFlags) = 0;
  /** Bind a buffer as storage input to shader module(s).
  *   A shader storage buffer is stored in the GPU's DDR memory 
     (or in shared system memory on systems which have a shared 
     memory architecture, like a mobile device).
     The GPU has read/write access.
  *   @param commandBuffer The command buffer
  *   @param buffer The input buffer
  *   @param binding The target binding
  *   @param shaderStageFlags The target shader module(s)
  */
  virtual void bindStorageBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                 uint32_t binding,
                                 ShaderStageFlags shaderStageFlags) = 0;
  /** Bind compute pipeline.
  *   The compute pipeline defines the GPU pipeline parameters to perform compute operations.
  *   @param cmdBuffer The command buffer
  *   @param computePipeline The compute pipeline.
  */
  virtual void bindComputePipeline(CommandBuffer *cmdBuffer,
                                   ComputePipeline *computePipeline) = 0;
  /** Bind graphics pipeline.
  *   The graphics pipeline defines the GPU pipeline parameters to perform graphics operations.
  *   @param cmdBuffer The command buffer
  *   @param computePipeline The graphics pipeline.
  */
  virtual void bindGraphicsPipeline(CommandBuffer *cmdBuffer,
                                    GraphicsPipeline *graphicsPipeline) = 0;
  /** Bind texture.
  *   If the texture has a built-in sampler, this also binds the sampler.
  *   @param cmdBuffer The command buffer
  *   @param texture The input texture
  *   @param set The descriptor set index
  */
  virtual void bindTexture(CommandBuffer *commandBuffer, Texture *texture,
                           uint32_t set) = 0;

  /** Bind texture to an image unit.
 *   This supports random-access reading and writing to the texture from the shader.
 *   @param cmdBuffer The command buffer
 *   @param texture The input texture
 *   @param set The descriptor set index
 */
  virtual void bindTextureAsImage(CommandBuffer* commandBuffer, Texture* texture,
      uint32_t set) = 0;

  /** Bind sampler.
      This allows the GPU shader module to sample a texture.
      When using a texture with a built-in sampler, this call is not necessary.
      @param cmdBuffer THe command buffer
      @param sampler The texture sampler
      @param set The descriptor set index
  */
  virtual void bindSampler(CommandBuffer* cmdBuffer, Sampler* sampler,
      uint32_t set) = 0;

  // TODO: copyBuffer: ToBuffer, copyBuffer: ToTexture, copyTexture: ToBuffer,
  // blit

  /** Draw primitives.
  *   The primitive type is speficied in the graphics pipeline object.
  *   This function supports instanced drawing: drawing multiple instances of each primitive.
  *   @param cmdBuffer The command buffer
  *   @param vertexCount The number of vertices
  *   @param instanceCount The number of instances
  *   @param firstVertex The starting vertex index
  *   @param firstInstance The starting instance index
  */
  virtual void draw(CommandBuffer *cmdBuffer, uint32_t vertexCount,
                    uint32_t instanceCount = 1, uint32_t firstVertex = 0,
                    uint32_t firstInstance = 0) = 0;
  /** Draw primitives.
  *   The primitive type is speficied in the graphics pipeline object.
  *   This function supports indexed drawing: specfying the vertices indirectly using indexes.
  *   It also supports instanced drawing.
  *   @param cmdBuffer The command buffer
  *   @param indexCount The number of indices
  *   @param instanceCount The number of instances
  *   @param firstIndex The starting index
  *   @param vertexOffset The offset added to the indices
  *   @param firstInstance The starting index of the first instance
  */
  virtual void drawIndexed(CommandBuffer *cmdBuffer, uint32_t indexCount,
                           uint32_t instanceCount = 1, uint32_t firstIndex = 0,
                           int32_t vertexOffset = 0,
                           uint32_t firstInstance = 0) = 0;
  /** Dispatch compute worker threads
  *   @param cmdBuffer The command buffer
  *   @param groupCountX, groupCountY, groupCountZ The number of groups (tensor)
  *   @param threadsPerGroupX, threadsPerGroupY, threadsPerGroupZ The number of threads per group (tensor)
  */
  virtual void dispatch(CommandBuffer *cmdBuffer, uint32_t groupCountX,
                        uint32_t groupCountY, uint32_t groupCountZ,
                        uint32_t threadsPerGroupX, uint32_t threadsPerGroupY,
                        uint32_t threadsPerGroupZ) = 0;
  /** Set the viewport
  *   This defines the mapping of view coordinates to NDC coordinates.
  *   @param cmdBuffer The command buffer
  *   @param rect The viewport rect
  */
  virtual void setViewport(CommandBuffer *cmdBuffer, Rect2D rect) = 0;

  /** Set the scissor rect
  *   @param cmdBuffer The command buffer
  *   @param rect The scissor rect
  */
  virtual void setScissor(CommandBuffer *cmdBuffer, Rect2D rect) = 0;

  /** Wait for the GPU to finish executing the command buffer.
  *   @param cmdBuffer The command buffer
  */
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
