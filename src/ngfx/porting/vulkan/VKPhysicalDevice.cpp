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
#include "ngfx/porting/vulkan/VKPhysicalDevice.h"
#include "ngfx/porting/vulkan/VKConfig.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include <algorithm>
using namespace ngfx;

void VKPhysicalDevice::getProperties() {
  vkGetPhysicalDeviceProperties(v, &deviceProperties);
  vkGetPhysicalDeviceFeatures(v, &deviceFeatures);
  vkGetPhysicalDeviceMemoryProperties(v, &deviceMemoryProperties);
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(v, &queueFamilyCount, nullptr);
  assert(queueFamilyCount > 0);
  queueFamilyProperties.resize(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(v, &queueFamilyCount,
                                           queueFamilyProperties.data());
  // Get list of supported extensions
  uint32_t extCount = 0;
  vkEnumerateDeviceExtensionProperties(v, nullptr, &extCount, nullptr);
  if (extCount > 0) {
    std::vector<VkExtensionProperties> extensions(extCount);
    if (vkEnumerateDeviceExtensionProperties(
            v, nullptr, &extCount, &extensions.front()) == VK_SUCCESS) {
      for (auto ext : extensions) {
        supportedExtensions.push_back(ext.extensionName);
      }
    }
  }
}
void VKPhysicalDevice::selectDevice(VkInstance instance) {
  VkResult vkResult;
  uint32_t gpuCount = 0;
  // Get number of available physical devices
  V(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
  assert(gpuCount > 0);
  // Enumerate devices
  std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
  V(vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data()));
  // Select GPU device
  v = physicalDevices[0];
  std::vector<VkPhysicalDeviceProperties> devicesProperties;
  devicesProperties.resize(physicalDevices.size());
  for (int j = 0; j < physicalDevices.size(); j++) {
    auto &device = physicalDevices[j];
    vkGetPhysicalDeviceProperties(device, &devicesProperties[j]);
  }
  auto containsDevice = [&](VkPhysicalDeviceType type,
                            VkPhysicalDevice &deviceResult) -> bool {
    for (int j = 0; j < devicesProperties.size(); j++) {
      auto &deviceProps = devicesProperties[j];
      if (deviceProps.deviceType == type) {
        deviceResult = physicalDevices[j];
        return true;
      }
    }
    return false;
  };
  VkPhysicalDevice physicalDeviceResult;
  if (containsDevice(PREFERRED_DEVICE_TYPE, physicalDeviceResult))
    v = physicalDeviceResult;
  else
    v = physicalDevices[0];
  vkGetPhysicalDeviceProperties(v, &deviceProperties);
  NGFX_LOG("selected device: %s", deviceProperties.deviceName);
}

void VKPhysicalDevice::chooseDepthFormat() {
  std::vector<VkFormat> depthFormatCandidates = {
      VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_D24_UNORM_S8_UINT,
      VK_FORMAT_D16_UNORM
  };

  for (auto &format : depthFormatCandidates) {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(v, format, &formatProps);
    // Format must support depth stencil attachment for optimal tiling
    if (formatProps.optimalTilingFeatures &
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      depthFormat = format;
      break;
    }
  }
}

uint32_t
VKPhysicalDevice::getMemoryType(uint32_t type,
                                VkMemoryPropertyFlags memoryPropertyFlags) {
  for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
    if ((type & 1) == 1) {
      auto &memoryType = deviceMemoryProperties.memoryTypes[i];
      if ((memoryType.propertyFlags & memoryPropertyFlags) ==
          memoryPropertyFlags) {
        return i;
      }
    }
    type >>= 1;
  }
  NGFX_ERR("Could not find a matching memory type");
  return 0;
}

void VKPhysicalDevice::create(VkInstance instance) {
  selectDevice(instance);
  getProperties();
  chooseDepthFormat();
}

VKPhysicalDevice::~VKPhysicalDevice() {}

bool VKPhysicalDevice::extensionSupported(std::string extension) {
  return (std::find(supportedExtensions.begin(), supportedExtensions.end(),
                    extension) != supportedExtensions.end());
}
