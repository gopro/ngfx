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

#include "MTLApplication.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/porting/metal/MTLGraphicsContext.h"
#include "ngfx/porting/metal/MTLCommandBuffer.h"
#include "ngfx/porting/metal/MTLSurface.h"
#include <functional>
using namespace ngfx;
using namespace std::placeholders;

extern std::function<void(void*)> appInit, appPaint, appUpdate;

MTLApplication::MTLApplication(const std::string& appName,
        int width, int height, bool enableDepthStencil, bool offscreen)
    : BaseApplication(appName, width, height, enableDepthStencil, offscreen) {}

void MTLApplication::init() {
    if (offscreen) BaseApplication::init();
    else {
        appInit = [&](void* view) {
            graphicsContext.reset(GraphicsContext::create(appName.c_str(), enableDepthStencil, true));
            MTLSurface surface;
            MTKView* mtkView = (MTKView*)view;
            surface.view = mtkView;
            if (w != -1 && h != -1) {
                [mtkView setFrame: NSMakeRect(0, 0, w, h)];
            }
            CGSize mtkViewSize = [mtkView drawableSize];
            surface.w = mtkViewSize.width;
            surface.h = mtkViewSize.height;
            graphicsContext->setSurface(&surface);
            graphics.reset(Graphics::create(graphicsContext.get()));
            onInit();
        };
        appPaint = [&](void* view) {
            paint();
        };
        appUpdate = [&](void*) { onUpdate(); };
    }
}

void MTLApplication::run() {
    init();
    const char* argv[] = {""};
    NSApplicationMain(1, argv);
}


void MTLApplication::paint() {
    auto ctx = mtl(graphicsContext.get());
    MTLCommandBuffer commandBuffer;
    commandBuffer.v = [ctx->mtlCommandQueue commandBuffer];
    onRecordCommandBuffer(&commandBuffer);
    if (!offscreen) {
        MTKView* mtkView = (MTKView*)mtl(ctx->surface)->view;
        [commandBuffer.v presentDrawable:mtkView.currentDrawable];
    }
    commandBuffer.commit();
    if (offscreen) graphics->waitIdle(&commandBuffer);
}
