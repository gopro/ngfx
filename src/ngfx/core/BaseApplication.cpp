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
#include <cstdio>
#include "ngfx/core/BaseApplication.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/core/FPSCounter.h"
using namespace ngfx;
using namespace std::placeholders;

BaseApplication::BaseApplication(const std::string& appName, int w, int h,
        bool enableDepthStencil, bool offscreen):
    appName(appName), w(w), h(h), enableDepthStencil(enableDepthStencil), offscreen(offscreen) {
        
    }

void BaseApplication::init() {
    auto& ctx = graphicsContext;
    ctx.reset(GraphicsContext::create(appName.c_str(), enableDepthStencil));
    if (offscreen) {
        Surface surface(w, h, true);
        graphicsContext->setSurface(&surface);
    }
    else { createWindow(); }
    graphics.reset(Graphics::create(ctx.get()));
    if (window) {
        window->onUpdate = std::bind(&BaseApplication::onUpdate, this);
        window->onPaint = std::bind(&BaseApplication::onPaint, this);
        window->onKey = std::bind(&BaseApplication::onKey, this, _1, _2);
        window->onScroll = std::bind(&BaseApplication::onScroll, this, _1, _2);
        window->onCursorPos = std::bind(&BaseApplication::onCursorPos, this, _1, _2);
        window->onMouseButton = std::bind(&BaseApplication::onMouseButton, this, _1, _2);
    }
    else if (offscreen) {
        uint32_t size = w * h * 4;
        outputTexture.reset(ngfx::Texture::create(ctx.get(), graphics.get(), nullptr, PIXELFORMAT_RGBA8_UNORM, size, w, h, 1, 1,
                ImageUsageFlags(IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT | IMAGE_USAGE_TRANSFER_DST_BIT | IMAGE_USAGE_COLOR_ATTACHMENT_BIT)));
        std::vector<ngfx::Framebuffer::Attachment> attachments = { { outputTexture.get() } };
        if (enableDepthStencil) {
            depthTexture.reset(ngfx::Texture::create(ctx.get(), graphics.get(), nullptr, ctx->depthFormat, size, w, h, 1, 1,
                IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
            attachments.push_back({ depthTexture.get() });
        }
        outputFramebuffer.reset(Framebuffer::create(ctx->device, ctx->defaultOffscreenRenderPass, attachments, w, h));
    }
    onInit();
    if (persistentCommandBuffers) recordCommandBuffers();
}

void BaseApplication::createWindow() {
    auto& ctx = graphicsContext;
    window.reset(Window::create(ctx.get(), appName.c_str(), [&](Window* thiz) {
        ctx->setSurface(thiz->surface);
    }, w, h));
}

void BaseApplication::run() {
    if (initOnce) { init(); initOnce = false; }
    FPSCounter fpsCounter;
    while (!window->shouldClose()) {
        drawFrame();
        fpsCounter.update();
    }
    close();
}

void BaseApplication::drawFrame() {
    if (initOnce) { init(); initOnce = false; }
    if (window) window->pollEvents();
    else if (offscreen) {
        onUpdate();
        onPaint();
    }
}

void BaseApplication::close() {
    auto commandBuffer = graphicsContext->drawCommandBuffer();
    graphics->waitIdle(commandBuffer);
}

void BaseApplication::recordCommandBuffers() {
    auto &ctx = graphicsContext;
    for (uint32_t j = 0; j < ctx->numDrawCommandBuffers; j++) {
        auto commandBuffer = ctx->drawCommandBuffer(j);
        commandBuffer->begin();
        ctx->currentImageIndex = j;
        onRecordCommandBuffer(commandBuffer);
        commandBuffer->end();
    }
}

void BaseApplication::onPaint() {
    paint();
}

void BaseApplication::paint() {
    auto &ctx = graphicsContext;
    if (!offscreen) ctx->swapchain->acquireNextImage();
    auto commandBuffer = ctx->drawCommandBuffer();
    if (!persistentCommandBuffers) {
        commandBuffer->begin();
        onRecordCommandBuffer(commandBuffer);
        commandBuffer->end();
    }
    ctx->queue->submit(commandBuffer);
    if (!offscreen) ctx->queue->present();
    else { graphics->waitIdle(commandBuffer); }
}
