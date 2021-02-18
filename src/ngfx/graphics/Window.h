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
#include <functional>
#include "ngfx/graphics/Surface.h"
#include "ngfx/input/InputMap.h"

namespace ngfx {
    class GraphicsContext;
    class Window {
    public:
        static Window* create(GraphicsContext* graphicsContext, const char* title, std::function<void(Window* thiz)> onWindowCreated,
            int w = DISPLAY_WIDTH, int h = DISPLAY_HEIGHT);
        virtual ~Window() {}
        virtual bool shouldClose() = 0;
        virtual void pollEvents() = 0;
        enum { DISPLAY_WIDTH = -1, DISPLAY_HEIGHT = -1 };
        int w, h;
        Surface* surface = nullptr;
        std::function<void()> onUpdate = nullptr, onPaint = nullptr;
        std::function<void(KeyCode code, InputAction action)> onKey = nullptr;
        std::function<void(double xoffset, double yoffset)> onScroll = nullptr;
        std::function<void(double x, double y)> onCursorPos = nullptr;
        std::function<void(MouseButton button, InputAction action)> onMouseButton = nullptr;
    };
};
