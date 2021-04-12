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
#include "ngfx/porting/vulkan/VKDebugMessenger.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/core/DebugUtil.h"
#include <sstream>
#include <string>
using namespace ngfx;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {

  std::stringstream debugMessage;
  debugMessage << "[" << pCallbackData->messageIdNumber << "]["
               << pCallbackData->pMessageIdName
               << "] : " << pCallbackData->pMessage;
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    NGFX_ERR("%s", debugMessage.str().c_str());
  } else
    NGFX_LOG("%s", debugMessage.str().c_str());
  return VK_FALSE;
}

void VKDebugMessenger::create(VkInstance instance,
                              VkDebugReportFlagsEXT flags) {
  this->instance = instance;
  VkResult vkResult;
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
      reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
  debugUtilsMessengerCI.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugUtilsMessengerCI.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugUtilsMessengerCI.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debugUtilsMessengerCI.pfnUserCallback = debugUtilsMessengerCallback;
  V(vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCI, nullptr,
                                   &v));
}
void VKDebugMessenger::destroy() {
  PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
      reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  VK_TRACE(vkDestroyDebugUtilsMessengerEXT(instance, v, nullptr));
}
