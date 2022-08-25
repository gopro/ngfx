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
#include <cstdint>

namespace ngfx {

/** \class Surface
 *
 *  This class provides the base class for an abstraction of a surface such as a 
 *  window surface or offscreen surface.  The difference between a 
 *  surface and a texture is that a texture supports random sampling, 
 *  and is typically stored in memory using a different layout to optimize 
 *  for sampling.  The user can either create a surface via the Window API 
 *  or wrap an existing surface that's created externally.
 */

class Surface {
public:
  Surface() {}
  /** Construct the surface object */
  Surface(uint32_t w, uint32_t h, bool offscreen = false)
      : w(w), h(h), offscreen(offscreen) {}
  /** Wrap an existing window handle */
  static Surface* createFromWindowHandle(uint32_t w, uint32_t h, void* handle);
  virtual ~Surface() {}
  /** The surface width */
  uint32_t w = 0,
  /** The surface height */
      h = 0;
  /** If true, the surface is offscreen */
  bool offscreen = false;
};
}; // namespace ngfx
