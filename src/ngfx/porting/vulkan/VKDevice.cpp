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
#include "ngfx/porting/vulkan/VKDevice.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
using namespace ngfx;

uint32_t VKDevice::getQueueFamilyIndex(VkQueueFlags queueFlags) {
    auto queueFamilyProperties = vkPhysicalDevice->queueFamilyProperties;
    if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
        //Try to find dedicated queue for compute
        for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
            auto &props = queueFamilyProperties[i];
            if ((props.queueFlags & queueFlags) && (!(props.queueFlags & VK_QUEUE_GRAPHICS_BIT))) {
                return i;
                break;
            }
        }
    }

    if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
        //Try to find dedicated queue for transfer
        for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
            auto &props = queueFamilyProperties[i];
            if ((props.queueFlags & queueFlags) && (!(props.queueFlags & VK_QUEUE_GRAPHICS_BIT)) &&
                    (!(props.queueFlags & VK_QUEUE_COMPUTE_BIT))) {
                return i;
                break;
            }
        }
    }

    // Return the first queue to support the requested flags
    for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
        if (queueFamilyProperties[i].queueFlags & queueFlags) {
            return i;
            break;
        }
    }
    NGFX_ERR("Could not find a matching queue family index");
    return 0;
}

void VKDevice::getQueueCreateInfos(VkQueueFlags requestedQueueTypes) {
    // Get queue family indices for the requested queue family types
    // Note that the indices may overlap depending on the implementation
    static const float defaultQueuePriority = 0.0f;
    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
    {
        queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    }
    else
    {
        queueFamilyIndices.graphics = VK_NULL_HANDLE;
    }

    // Compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
    {
        queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
        {
            // Found dedicated compute queue
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
    }
    else
    {
        // Use the same queue
        queueFamilyIndices.compute = queueFamilyIndices.graphics;
    }

    // Transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
    {
        queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
        if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
        {
            //Found dedicated transfer queue
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
    }
    else
    {
        // Use the same queue
        queueFamilyIndices.transfer = queueFamilyIndices.graphics;
    }
}

void VKDevice::getDeviceExtensions() {
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    // Enable the debug marker extension if it is present
    if (vkPhysicalDevice->extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME) &&
            vkPhysicalDevice->extensionSupported(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
        deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        deviceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        enableDebugMarkers = true;
    }
}
void VKDevice::create(VKPhysicalDevice* vkPhysicalDevice) {
    VkResult vkResult;
    this->vkPhysicalDevice = vkPhysicalDevice;
    getQueueCreateInfos();
    getDeviceExtensions();

    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    enabledDeviceExtensions.resize(deviceExtensions.size());
    for (uint32_t j = 0; j<deviceExtensions.size(); j++) enabledDeviceExtensions[j] = deviceExtensions[j].c_str();
    createInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
    V(vkCreateDevice(vkPhysicalDevice->v, &createInfo, nullptr, &v));
}
void VKDevice::waitIdle() {
    VkResult vkResult;
    V(vkDeviceWaitIdle(v));
}
VKDevice::~VKDevice() {
    if (v) VK_TRACE(vkDestroyDevice(v, nullptr));
}
