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
#include "ngfx/porting/d3d/D3DWindow.h"
#include <windows.h>

namespace ngfx {
class WWindow : public D3DWindow {
public:
  void create(GraphicsContext *graphicsContext, const char *title,
              std::function<void(Window *thiz)> onWindowCreated, int w, int h);
  virtual bool shouldClose();
  virtual void pollEvents();
  virtual ~WWindow();
  virtual void destroySurface();

private:
  MSG msg{};
  void createWindow(int w, int h, const char *title);
  void getFramebufferSize(int *w, int *h);
  struct Monitor {
    DISPLAY_DEVICEW adapter, display;
  };
  Monitor getPrimaryMonitor();
  struct VideoMode {
    int width, height;
  };
  VideoMode getVideoMode(const Monitor &monitor);
  void registerWindowClass();
};
inline WWindow *w(Window *window) { return (WWindow *)window; }
} // namespace ngfx