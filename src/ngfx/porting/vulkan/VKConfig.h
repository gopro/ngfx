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
#include <vulkan/vulkan.h>
//#define VK_FLIP_Y

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#define VK_SURFACE_EXTENSION_NAMES VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#define VK_SURFACE_EXTENSION_NAMES VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#define VK_SURFACE_EXTENSION_NAMES VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR) && defined(VK_USE_PLATFORM_XLIB_KHR)
#define VK_SURFACE_EXTENSION_NAMES                                             \
  VK_KHR_XCB_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#define VK_SURFACE_EXTENSION_NAMES VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
#define VK_SURFACE_EXTENSION_NAMES VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_IOS_MVK)
#define VK_SURFACE_EXTENSION_NAMES VK_MVK_IOS_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
#define VK_SURFACE_EXTENSION_NAMES VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_METAL_EXT)
#define VK_SURFACE_EXTENSION_NAMES VK_EXT_METAL_SURFACE_EXTENSION_NAME
#endif

#define PREFERRED_DEVICE_TYPE                                                  \
  VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU // VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
#define ORIGIN_BOTTOM_LEFT
