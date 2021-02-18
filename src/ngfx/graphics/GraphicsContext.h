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
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/Graphics.h"
#include "ngfx/graphics/Framebuffer.h"
#include "ngfx/graphics/PipelineCache.h"
#include "ngfx/graphics/Queue.h"
#include "ngfx/compute/ComputePass.h"
#include "ngfx/graphics/RenderPass.h"
#include "ngfx/graphics/Swapchain.h"
#include "ngfx/graphics/Device.h"
#include "ngfx/graphics/Surface.h"
#include <vector>
#include <optional>

namespace ngfx {
    class GraphicsContext {
    public:
        static GraphicsContext* create(const char* appName, bool enableDepthStencil = false, bool debug = true);
        virtual ~GraphicsContext() {}
        virtual void setSurface(Surface* surface) = 0;
        virtual void beginRenderPass(CommandBuffer* commandBuffer, Graphics* graphics) {
            auto framebuffer = swapchainFramebuffers[currentImageIndex];
            uint32_t w = framebuffer->w, h = framebuffer->h;
            graphics->beginRenderPass(commandBuffer, defaultRenderPass, framebuffer, clearColor);
            graphics->setViewport(commandBuffer, { 0, 0, w, h });
            graphics->setScissor(commandBuffer, { 0, 0, w, h });
        }
        virtual void beginOffscreenRenderPass(CommandBuffer* commandBuffer, Graphics* graphics, Framebuffer* outputFramebuffer) {
            graphics->beginRenderPass(commandBuffer, defaultOffscreenRenderPass, outputFramebuffer, clearColor);
            graphics->setViewport(commandBuffer, { 0, 0, outputFramebuffer->w, outputFramebuffer->h });
            graphics->setScissor(commandBuffer, { 0, 0, outputFramebuffer->w, outputFramebuffer->h });
        }
        virtual void endRenderPass(CommandBuffer* commandBuffer, Graphics* graphics) {
            graphics->endRenderPass(commandBuffer);
        }
        virtual void endOffscreenRenderPass(CommandBuffer* commandBuffer, Graphics* graphics) {
            graphics->endRenderPass(commandBuffer);
        }
        virtual void submit(CommandBuffer* commandBuffer) {
            queue->submit(commandBuffer);
        }
        Device* device;
        uint32_t numDrawCommandBuffers = 0;
        virtual CommandBuffer* drawCommandBuffer(int32_t index = -1) = 0;
        virtual CommandBuffer* copyCommandBuffer() = 0;
        virtual CommandBuffer* computeCommandBuffer() = 0;

        struct AttachmentDescription {
            bool operator ==(const AttachmentDescription &rhs) const {
                return rhs.format == format &&
                       rhs.initialLayout == initialLayout &&
                       rhs.finalLayout == finalLayout;
            }
            PixelFormat format;
            std::optional<ImageLayout> initialLayout, finalLayout;
        };

        struct RenderPassConfig {
            bool operator ==(const RenderPassConfig &rhs) const {
                return rhs.colorAttachmentDescriptions == colorAttachmentDescriptions &&
                       rhs.depthStencilAttachmentDescription == depthStencilAttachmentDescription &&
                       rhs.enableDepthStencilResolve == enableDepthStencilResolve &&
                       rhs.numSamples == numSamples;
            };
            uint32_t numColorAttachments() const {
                return uint32_t(colorAttachmentDescriptions.size());
            }
            std::vector<AttachmentDescription> colorAttachmentDescriptions;
            std::optional<AttachmentDescription> depthStencilAttachmentDescription;
            bool enableDepthStencilResolve = false;
            uint32_t numSamples = 1;
        };
        virtual RenderPass* getRenderPass(RenderPassConfig config) = 0;

        std::vector<Framebuffer*> swapchainFramebuffers;
        Queue* queue = nullptr;
        RenderPass *defaultRenderPass = nullptr, *defaultOffscreenRenderPass = nullptr;
        Swapchain* swapchain = nullptr;
        Surface* surface = nullptr;
        uint32_t currentImageIndex = 0;
        std::vector<Fence*> frameFences;
        Fence* computeFence = nullptr;
        Semaphore *presentCompleteSemaphore = nullptr, *renderCompleteSemaphore = nullptr;
        PipelineCache *pipelineCache = nullptr;
        PixelFormat surfaceFormat = PIXELFORMAT_UNDEFINED, defaultOffscreenSurfaceFormat = PIXELFORMAT_UNDEFINED,
            depthFormat = PIXELFORMAT_UNDEFINED;
        glm::vec4 clearColor = glm::vec4(0.0f);
    protected:
        bool debug = false, enableDepthStencil = false;
    };
};
