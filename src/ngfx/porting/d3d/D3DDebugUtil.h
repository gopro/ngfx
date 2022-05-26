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
#include "ngfx/core/DebugUtil.h"
#include <d3d12.h>
#include <system_error>

/** Trace all Direct3D calls to log output */
const bool D3D_ENABLE_TRACE = false;
const bool DEBUG_SHADERS = true;
// Enabling GPU validation slows down performance but it's useful for debugging
const bool ENABLE_GPU_VALIDATION = false;

#define D3D_TRACE(func)                                                        \
  {                                                                            \
    if (D3D_ENABLE_TRACE)                                                      \
      NGFX_LOG("%s", #func);                                                   \
    func;                                                                      \
  }

#define V0(func, fmt, ...)                                                     \
  {                                                                            \
    if (D3D_ENABLE_TRACE)                                                      \
      NGFX_LOG("%s", #func);                                                   \
    hResult = func;                                                            \
    if (FAILED(hResult)) {                                                     \
      NGFX_ERR("%s failed: 0x%08X %s " fmt, #func, hResult,                    \
               std::system_category().message(hResult).c_str(),                \
               ##__VA_ARGS__);                                                 \
    }                                                                          \
  }

#define V(func) V0(func, "")
