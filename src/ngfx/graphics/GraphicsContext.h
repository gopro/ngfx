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
#include "ngfx/compute/ComputePass.h"
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/Device.h"
#include "ngfx/graphics/Framebuffer.h"
#include "ngfx/graphics/Graphics.h"
#include "ngfx/graphics/PipelineCache.h"
#include "ngfx/graphics/Queue.h"
#include "ngfx/graphics/RenderPass.h"
#include "ngfx/graphics/Surface.h"
#include "ngfx/graphics/Swapchain.h"
#include <functional>
#include <optional>
#include <vector>

namespace ngfx {
/** \class GraphicsContext
*
*  This class provides an abstraction of a graphics context, as well as 
*  various utility functions.  The process can have more than one graphics context.
*  Each graphics context can have either an onscreen or offscreen surface, and it also 
*  supports headless mode (no surface attachment), which is especially useful for compute applications.
*/
class GraphicsContext {
public:
  using OnSelectDepthStencilFormats = std::function<void(
      const std::vector<PixelFormat>& depthStencilFormatCandidates,
      PixelFormat& depthFormat,
      PixelFormat& depthStencilFormat
  )>;
  /** Create the graphics context
   *  @param appName The application name
   *  @param enableDepthStencil Enable depth/stencil buffer
   *  @bool debug Enable debugging and validation layers
   *  @param onSelectDepthStencilFormats An optional callback that lets the user override the depth / depthStencil format
   */
  static GraphicsContext *create(const char *appName,
                                 bool enableDepthStencil = false,
                                 bool debug = true,
                                 OnSelectDepthStencilFormats onSelectDepthStencilFormats = nullptr);
  virtual ~GraphicsContext() {}
  /** Set the surface for the graphics context 
   *  This can be an offscreen or onscreen surface.
   *  Also, if the user passes nullptr, then the graphics context will work in 
   *  "headless" mode.  One use case for headless mode is an application that only does compute operations.
   *   @param surface The surface attachment (or nullptr for headless mode) */
  virtual void setSurface(Surface *surface) = 0;
  /** Begin a render pass: for drawing to the main surface attached to the context.
   *  This is a helper function that also sets the viewport and scissor rect.
      @param commandBuffer The command buffer
      @param graphics      The graphics interface for recording graphics commands to the command buffer */
  virtual void beginRenderPass(CommandBuffer *commandBuffer,
                               Graphics *graphics) {
    if (currentImageIndex == -1)
        currentImageIndex = 0;
    auto framebuffer = swapchainFramebuffers[currentImageIndex];
    uint32_t w = framebuffer->w, h = framebuffer->h;
    graphics->beginRenderPass(commandBuffer, defaultRenderPass, framebuffer,
                              clearColor);
    graphics->setViewport(commandBuffer, {0, 0, w, h});
    graphics->setScissor(commandBuffer, {0, 0, w, h});
  }
  /** Begin an offscreen render pass.
  *   This is a helper function for drawing to a texture or to 
      one or more attachments via a framebuffer object.
  *   @param commandBuffer The command buffer
  *   @param graphics The graphics interface for recording graphics commands to the command buffer
  *   @param outputFramebuffer The framebuffer object which provides a set of one or more attachments
  *   to draw to */
  virtual void beginOffscreenRenderPass(CommandBuffer *commandBuffer,
                                        Graphics *graphics,
                                        Framebuffer *outputFramebuffer) {
    graphics->beginRenderPass(commandBuffer, defaultOffscreenRenderPass,
                              outputFramebuffer, clearColor);
    graphics->setViewport(commandBuffer,
                          {0, 0, outputFramebuffer->w, outputFramebuffer->h});
    graphics->setScissor(commandBuffer,
                         {0, 0, outputFramebuffer->w, outputFramebuffer->h});
  }
  /** End render pass.  Every call to beginRenderPass should be accompanied by endRenderPass.
      @param commandBuffer The command buffer for recording graphics commands
      @param graphics      The graphics interface for recording graphics commands to the command buffer */
  virtual void endRenderPass(CommandBuffer *commandBuffer, Graphics *graphics) {
    graphics->endRenderPass(commandBuffer);
  }
  /** End offscreen render pass.  Every call to beginOffscreenRenderPass should be accompanied by endOffscreenRenderPass.
      @param commandBuffer The command buffer for recording graphics commands
      @param graphics      The graphics interface for recording graphics commands to the command buffer */

  virtual void endOffscreenRenderPass(CommandBuffer *commandBuffer,
                                      Graphics *graphics) {
    graphics->endRenderPass(commandBuffer);
  }
  /** Submit the command buffer to the graphics queue */
  virtual void submit(CommandBuffer *commandBuffer) {
    queue->submit(commandBuffer);
  }
  Device *device;
  uint32_t numDrawCommandBuffers = 0;
  virtual CommandBuffer *drawCommandBuffer(int32_t index = -1) = 0;
  virtual CommandBuffer *copyCommandBuffer() = 0;
  virtual CommandBuffer *computeCommandBuffer() = 0;

  /** \struct AttachmentDescription
   *
   *  This struct defines the description for an attachment, including the initial and final layout,
   *  load operation, and store operation */
  struct AttachmentDescription {
      bool operator==(const AttachmentDescription& rhs) const {
          return rhs.format == format && rhs.initialLayout == initialLayout &&
              rhs.finalLayout == finalLayout && rhs.loadOp == loadOp && rhs.storeOp == storeOp;
      }
      PixelFormat format;
      std::optional<ImageLayout> initialLayout, finalLayout;
      AttachmentLoadOp loadOp;
      AttachmentStoreOp storeOp;
  };

  /** \struct RenderPassConfig
   *
   *  This struct defines a render pass configuration including the color attachments, 
   *  depth stencil attachment (optional), and number of samples when using multisampling */
  struct RenderPassConfig {
    bool operator==(const RenderPassConfig &rhs) const {
      return rhs.colorAttachmentDescriptions == colorAttachmentDescriptions &&
             rhs.depthStencilAttachmentDescription ==
                 depthStencilAttachmentDescription &&
             rhs.enableDepthStencilResolve == enableDepthStencilResolve &&
             rhs.numSamples == numSamples;
    };
    /** Get the number of color attachments */
    uint32_t numColorAttachments() const {
      return uint32_t(colorAttachmentDescriptions.size());
    }
    /** The color attachments */
    std::vector<AttachmentDescription> colorAttachmentDescriptions;
    /** The depth stencil attachment (optional) */
    std::optional<AttachmentDescription> depthStencilAttachmentDescription;
    /** Enable multisampling on the depth / stencil buffer */
    bool enableDepthStencilResolve = false;
    /** The number of samples when using multisampling (set to 1 when not using multisampling) */
    uint32_t numSamples = 1;
  };
  /** Get a render pass object that supports a given configuration */
  virtual RenderPass *getRenderPass(RenderPassConfig config) = 0;

  std::vector<Framebuffer *> swapchainFramebuffers;
  Queue *queue = nullptr;
  RenderPass *defaultRenderPass = nullptr,
             *defaultOffscreenRenderPass = nullptr;
  Swapchain *swapchain = nullptr;
  Surface *surface = nullptr;
  int32_t currentImageIndex = -1;
  std::vector<Fence *> frameFences;
  Fence *computeFence = nullptr;
  Fence* offscreenFence = nullptr;
  Semaphore *presentCompleteSemaphore = nullptr,
            *renderCompleteSemaphore = nullptr;
  PipelineCache *pipelineCache = nullptr;
  PixelFormat surfaceFormat = PIXELFORMAT_UNDEFINED,
              defaultOffscreenSurfaceFormat = PIXELFORMAT_UNDEFINED,
              depthFormat = PIXELFORMAT_UNDEFINED,
              depthStencilFormat = PIXELFORMAT_UNDEFINED;
  glm::vec4 clearColor = glm::vec4(0.0f);

protected:
  bool debug = false, enableDepthStencil = false;
  OnSelectDepthStencilFormats onSelectDepthStencilFormats;
};
}; // namespace ngfx
