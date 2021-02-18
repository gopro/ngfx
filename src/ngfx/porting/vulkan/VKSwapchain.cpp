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
#include "ngfx/porting/vulkan/VKSwapchain.h"
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/porting/vulkan/VKSemaphore.h"
#include "ngfx/core/Util.h"
#include "ngfx/porting/vulkan/VKDevice.h"
#include "ngfx/graphics/Config.h"
using namespace ngfx;

#ifdef ENABLE_VSYNC
static const VkPresentModeKHR PREFERRED_PRESENT_MODE = VK_PRESENT_MODE_FIFO_KHR;
#else
static const VkPresentModeKHR PREFERRED_PRESENT_MODE = VK_PRESENT_MODE_MAILBOX_KHR;
#endif

void VKSwapchain::chooseNumSwapchainImages() {
    numImages = PREFERRED_NUM_SWAPCHAIN_IMAGES;
    if ((surfaceCaps.maxImageCount > 0) && (numImages > surfaceCaps.maxImageCount)) {
        numImages = surfaceCaps.maxImageCount;
    }
}
void VKSwapchain::chooseSurfaceFormat() {
    for (auto & fmt : surfaceFormats) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_UNORM && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = fmt;
            return;
        }
    }
    surfaceFormat = surfaceFormats[0];
}
void VKSwapchain::getSurfaceCapabilities(VkPhysicalDevice physicalDevice) {
    VkResult vkResult;
    V(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface->v, &surfaceCaps));
}
void VKSwapchain::getSurfaceFormats(VkPhysicalDevice physicalDevice) {
    VkResult vkResult;
    uint32_t surfaceFormatCount;
    V(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface->v, &surfaceFormatCount, nullptr));
    assert(surfaceFormatCount > 0);
    surfaceFormats.resize(surfaceFormatCount);
    V(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface->v, &surfaceFormatCount, surfaceFormats.data()));
}

void VKSwapchain::choosePresentMode() {
    if (!Util::contains(presentModes, PREFERRED_PRESENT_MODE)) {
        NGFX_ERR("present mode: %d not supported", PREFERRED_PRESENT_MODE);
    }
    else presentMode = PREFERRED_PRESENT_MODE;
}

void VKSwapchain::getPresentModes(VkPhysicalDevice physicalDevice) {
    VkResult vkResult;
    uint32_t presentModeCount;
    V(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface->v, &presentModeCount, nullptr));
    assert(presentModeCount > 0);
    presentModes.resize(presentModeCount);
    V(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface->v, &presentModeCount, presentModes.data()));
}

void VKSwapchain::chooseCompositeAlphaMode() {
    //Choose the first composite alpha format available
    std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (auto& compositeAlphaFlag : compositeAlphaFlags) {
        if (surfaceCaps.supportedCompositeAlpha & compositeAlphaFlag) {
            compositeAlpha = compositeAlphaFlag;
            break;
        };
    }
}

void VKSwapchain::choosePreTransform() {
    if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else {
        preTransform = surfaceCaps.currentTransform;
    }
}
void VKSwapchain::createSwapchain(const VKDevice& vkDevice, uint32_t w, uint32_t h) {
    VkResult vkResult;
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.surface = surface->v;
    createInfo.minImageCount = numImages;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = surfaceCaps.currentExtent;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
    createInfo.imageArrayLayers = 1;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = NULL;
    createInfo.presentMode = presentMode;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    createInfo.clipped = VK_TRUE;
    createInfo.compositeAlpha = compositeAlpha;

    // Enable transfer source on swap chain images if supported
    if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    V(vkCreateSwapchainKHR(vkDevice.v, &createInfo, nullptr, &v))
}
void VKSwapchain::getSwapchainImages(VkDevice device) {
    VkResult vkResult;
    uint32_t imageCount;
    V(vkGetSwapchainImagesKHR(device, v, &imageCount, NULL));
    images.resize(imageCount);
    V(vkGetSwapchainImagesKHR(device, v, &imageCount, images.data()));
}
void VKSwapchain::getQueueFamilyProperties(const VKPhysicalDevice &vkPhysicalDevice) {
    VkResult vkResult;
    uint32_t queueFamilyCount = uint32_t(vkPhysicalDevice.queueFamilyProperties.size());
    queueFamilyProperties.resize(queueFamilyCount);
    auto physicalDevice = vkPhysicalDevice.v;
    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++) {
        auto& queueFamilyProps = queueFamilyProperties[queueFamilyIndex];
        V(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface->v, &queueFamilyProps.supportsPresent));
    }
}
void VKSwapchain::createSwapchainImageViews(const VKDevice& vkDevice) {
    uint32_t imageCount = uint32_t(images.size());
    imageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        auto& imageView = imageViews[i];
        imageView.create(vkDevice.v, images[i], VK_IMAGE_VIEW_TYPE_2D, surfaceFormat.format);
    }
}
VKSwapchain::VKSwapchain(VKGraphicsContext* ctx, VKSurface* surface) {
    this->ctx = ctx;
    auto &vkDevice = ctx->vkDevice;
    this->device = vkDevice.v;
    this->surface = surface;
    chooseNumSwapchainImages();
    auto physicalDevice = vkDevice.vkPhysicalDevice->v;
    getSurfaceCapabilities(physicalDevice);
    getSurfaceFormats(physicalDevice);
    getPresentModes(physicalDevice);
    getQueueFamilyProperties(*vkDevice.vkPhysicalDevice);
    assert(queueFamilyProperties[vkDevice.queueFamilyIndices.graphics].supportsPresent);
    chooseSurfaceFormat();
    choosePresentMode();
    chooseCompositeAlphaMode();
    choosePreTransform();
    createSwapchain(vkDevice, surface->w, surface->h);
    getSwapchainImages(vkDevice.v);
    createSwapchainImageViews(vkDevice);
}

VKSwapchain::~VKSwapchain() {
    if (v) VK_TRACE(vkDestroySwapchainKHR(device, v, nullptr));
    surface->destroy();
};

void VKSwapchain::acquireNextImage() {
    VkResult vkResult;
    Semaphore* semaphore = ctx->presentCompleteSemaphore;
    uint32_t* imageIndex = &ctx->currentImageIndex;
    V(vkAcquireNextImageKHR(device, v, UINT64_MAX, vk(semaphore)->v, VK_NULL_HANDLE, imageIndex));
    auto waitFence = ctx->frameFences[ctx->currentImageIndex];
    waitFence->wait();
    waitFence->reset();
}
 
