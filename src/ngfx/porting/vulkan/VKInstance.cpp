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
#include "ngfx/porting/vulkan/VKInstance.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/porting/vulkan/VKConfig.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include <cstring>
using namespace ngfx;

void VKInstance::create(const char *appName, const char *engineName,
                        uint32_t apiVersion, bool enableValidation) {
  VkResult vkResult;
  this->settings.enableValidation = enableValidation;

  appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName;
  appInfo.pEngineName = engineName;
  appInfo.apiVersion = apiVersion;

  // Get instance layer properties
  uint32_t instanceLayerCount;
  vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
  instanceLayerProperties.resize(instanceLayerCount);
  vkEnumerateInstanceLayerProperties(&instanceLayerCount,
                                     instanceLayerProperties.data());

  // Set instance layers
  if (settings.enableValidation) {
    const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
    // Check if this layer is available
    bool validationLayerPresent = hasInstanceLayer(validationLayerName);
    if (validationLayerPresent) {
      instanceLayers.push_back(validationLayerName);
    } else {
      NGFX_ERR("Validation layer VK_LAYER_KHRONOS_validation not found");
    }
  }

  // Set instance extensions
  instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};

  std::vector<const char *> surfaceExtensions = {VK_SURFACE_EXTENSION_NAMES};
  for (auto &ext : surfaceExtensions)
    instanceExtensions.push_back(ext);
  if (instanceExtensions.size() > 0) {
    if (settings.enableValidation) {
      instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
  }

  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.ppEnabledExtensionNames = instanceExtensions.data();
  createInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
  createInfo.ppEnabledLayerNames = instanceLayers.data();
  createInfo.enabledLayerCount = (uint32_t)instanceLayers.size();

  V(vkCreateInstance(&createInfo, nullptr, &v));
}

bool VKInstance::hasInstanceLayer(const char *name) {
  for (VkLayerProperties &props : instanceLayerProperties) {
    if (strcmp(props.layerName, name) == 0) {
      return true;
    }
  }
  return false;
}

VKInstance::~VKInstance() {
  if (v)
    VK_TRACE(vkDestroyInstance(v, nullptr));
}
