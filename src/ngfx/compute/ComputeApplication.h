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
#include "ngfx/graphics/GraphicsContext.h"
#include "ngfx/graphics/Window.h"

namespace ngfx {
    class ComputeApplication {
    public:
        ComputeApplication(const std::string& name, int w = 0, int h = 0);
        virtual ~ComputeApplication() {}
        virtual void onInit() {}
        virtual void onRecordCommandBuffer(CommandBuffer* commandBuffer) {}
        virtual void onUpdate() {}
        virtual void run();
    protected:
        virtual void init();
        virtual void close();
        virtual void doCompute(CommandBuffer* commandBuffer);
        virtual void onComputeFinished() {}
        virtual void recordCommandBuffer(CommandBuffer* commandBuffer);
        std::unique_ptr<Graphics> graphics;
        std::unique_ptr<Window> window;
        std::unique_ptr<GraphicsContext> graphicsContext;
        std::string appName;
        int w, h;
    };
};
