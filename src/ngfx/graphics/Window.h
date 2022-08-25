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
#include "ngfx/graphics/Surface.h"
#include "ngfx/input/InputMap.h"
#include <functional>

namespace ngfx {
class GraphicsContext;
/** \class Window
 *
 *  The window class provides an abstraction class for window support.
 *  It provides APIs for creating a window and receiving window events including 
 *  update events, key events, and mouse events.
 */

class Window {
public:
  /** Create a window */
  static Window *create(GraphicsContext *graphicsContext, const char *title,
                        std::function<void(Window *thiz)> onWindowCreated,
                        int w = DISPLAY_WIDTH, int h = DISPLAY_HEIGHT);
  virtual ~Window() {}
  /** Poll if the user requested the window to close */
  virtual bool shouldClose() = 0;
  /** Poll for new events (non-blocking) */
  virtual void pollEvents() = 0;
  enum { DISPLAY_WIDTH = -1, DISPLAY_HEIGHT = -1 };
  int w, /**< The window width */
      h; /**< The window height */
  Surface *surface = nullptr; /**< The window surface */
  /**< An optional callback that will be called on window update events */
  std::function<void()> onUpdate = nullptr,
  /**< An optional callback that will be called on window paint events */
      onPaint = nullptr;
  /**< An optional callback that will be called on key events */
  std::function<void(KeyCode code, InputAction action)> onKey = nullptr;
  /**< An optional callback that will be called on scroll events */
  std::function<void(double xoffset, double yoffset)> onScroll = nullptr;
  /**< An optional callback that will be called when the mouse cursor position changes */
  std::function<void(double x, double y)> onCursorPos = nullptr;
  /**< An optional callback that will be called on mouse button events */
  std::function<void(MouseButton button, InputAction action)> onMouseButton =
      nullptr;
};
}; // namespace ngfx
