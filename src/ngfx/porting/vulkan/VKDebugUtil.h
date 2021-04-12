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
#include <cassert>
#include <cstdarg>
#include <vulkan/vulkan.h>
/** VK_TRACE all Vulkan calls to log output */
const int VK_ENABLE_TRACE = 0;

namespace ngfx {

struct VKDebugUtil {
    static const char *VkResultToString(VkResult errorCode);
};

}; // namespace ngfx

#define VK_TRACE(func)                                                         \
  {                                                                            \
    if (VK_ENABLE_TRACE)                                                       \
      NGFX_LOG("%s", #func);                                                   \
    func;                                                                      \
  }

#define V(func)                                                                \
  {                                                                            \
    if (VK_ENABLE_TRACE)                                                       \
      NGFX_LOG("%s", #func);                                                   \
    vkResult = func;                                                           \
    if (vkResult != VK_SUCCESS) {                                              \
      NGFX_ERR("%s failed: %s", #func,                                         \
               VKDebugUtil::VkResultToString(vkResult));                       \
    }                                                                          \
  }
