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
#include "ngfx/graphics/Swapchain.h"
#include "ngfx/porting/vulkan/VKDevice.h"
#include "ngfx/porting/vulkan/VKImageView.h"
#include "ngfx/porting/vulkan/VKPhysicalDevice.h"
#include "ngfx/porting/vulkan/VKSurface.h"
#include "ngfx/porting/vulkan/VKUtil.h"

namespace ngfx {
class VKGraphicsContext;
class VKSwapchain : public Swapchain {
public:
  VKSwapchain(VKGraphicsContext *ctx, VKSurface *surface);
  virtual ~VKSwapchain();
  virtual void acquireNextImage();
  VkSurfaceCapabilitiesKHR surfaceCaps;
  VKSurface *surface;
  VkSwapchainKHR v = VK_NULL_HANDLE;
  VkSurfaceFormatKHR surfaceFormat;
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  std::vector<VkPresentModeKHR> presentModes;
  VkPresentModeKHR presentMode;
  VkCompositeAlphaFlagBitsKHR compositeAlpha;
  VkSurfaceTransformFlagsKHR preTransform;
  std::vector<VkImage> images;
  std::vector<VKImageView> imageViews;
  struct queueFamilyProperties {
    VkBool32 supportsPresent = VK_FALSE;
  };
  std::vector<queueFamilyProperties> queueFamilyProperties;
  VkSwapchainCreateInfoKHR createInfo;

private:
  void chooseCompositeAlphaMode();
  void chooseSurfaceFormat();
  void chooseNumSwapchainImages();
  void choosePresentMode();
  void choosePreTransform();
  void createSwapchain(const VKDevice &vkDevice, uint32_t w, uint32_t h);
  void getPresentModes(VkPhysicalDevice physicalDevice);
  void getQueueFamilyProperties(const VKPhysicalDevice &vkPhysicalDevice);
  void getSurfaceCapabilities(VkPhysicalDevice physicalDevice);
  void getSwapchainImages(VkDevice device);
  void createSwapchainImageViews(const VKDevice &device);
  void getSurfaceFormats(VkPhysicalDevice physicalDevice);
  VKGraphicsContext *ctx;
  VkDevice device;
};
VK_CAST(Swapchain);
} // namespace ngfx
