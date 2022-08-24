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

 /** \module GraphicsCore
 *
 *  This module provides various enums as a platform abstraction API
 */

#include <cstdint>
#include <string>
#ifdef NGFX_GRAPHICS_BACKEND_VULKAN
#include "ngfx/porting/vulkan/VKGraphicsCore.h"
#endif
#ifdef NGFX_GRAPHICS_BACKEND_DIRECT3D12
#include "ngfx/porting/d3d/D3DGraphicsCore.h"
#endif
#ifdef NGFX_GRAPHICS_BACKEND_METAL
#include "ngfx/porting/metal/MTLGraphicsCore.h"
#endif

namespace ngfx {
typedef uint32_t Flags;
typedef Flags PipelineStageFlags;
typedef Flags ShaderStageFlags;
typedef Flags FenceCreateFlags;
typedef Flags ImageUsageFlags;
typedef Flags ColorComponentFlags;
typedef Flags BufferUsageFlags;
struct Rect2D {
  int32_t x, y;
  uint32_t w, h;
};
} // namespace ngfx
