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
#include "ngfx/graphics/GraphicsContext.h"
#ifdef NGFX_GRAPHICS_BACKEND_VULKAN
#include "ngfx/porting/vulkan/VKWindow.h"
#endif
#include <GLFW/glfw3.h>

namespace ngfx {
#ifdef NGFX_GRAPHICS_BACKEND_VULKAN
class GLFWWindow : public VKWindow {
#else
class GLFWWindow {
#endif
public:
  void create(GraphicsContext *graphicsContext, const char *title,
              std::function<void(Window *thiz)> setWindow, int w, int h);
  virtual bool shouldClose();
  virtual void pollEvents();
  virtual ~GLFWWindow();
  GLFWwindow *v;

protected:
};
inline GLFWWindow *glfw(Window *window) { return (GLFWWindow *)window; }
} // namespace ngfx
