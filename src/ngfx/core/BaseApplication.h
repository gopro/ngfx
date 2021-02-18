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
#include "ngfx/input/InputListener.h"
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/GraphicsContext.h"
#include "ngfx/graphics/Graphics.h"
#include "ngfx/graphics/Window.h"
#include <memory>
#include <string>

namespace ngfx {
    class BaseApplication : public InputListener {
    public:
        BaseApplication(const std::string& appName,
            int w = Window::DISPLAY_WIDTH,
            int h = Window::DISPLAY_HEIGHT,
            bool enableDepthStencil = false,
            bool offscreen = false);
        virtual ~BaseApplication() {}

        virtual void init();
        virtual void createWindow();
        virtual void paint();
        virtual void recordCommandBuffers();
        virtual void close();

        virtual void onInit() {}
        virtual void onRecordCommandBuffer(CommandBuffer* commandBuffer) = 0;
        virtual void onUpdate() {}
        virtual void onPaint();
        virtual void run();
        virtual void drawFrame();

        std::unique_ptr<Graphics> graphics;
        std::unique_ptr<Window> window;
        std::unique_ptr<GraphicsContext> graphicsContext;
        std::string appName;
        int w, h;
        bool enableDepthStencil = false;
        bool offscreen = false;
        bool persistentCommandBuffers = true;
    protected:
        bool initOnce = true;
        std::unique_ptr<ngfx::Texture> outputTexture, depthTexture;
        std::unique_ptr<Framebuffer> outputFramebuffer;
    };
};
