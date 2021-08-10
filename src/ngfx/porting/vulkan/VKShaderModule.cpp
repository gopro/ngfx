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
#include "ngfx/porting/vulkan/VKShaderModule.h"
#include "ngfx/core/File.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/porting/vulkan/VKDevice.h"
using namespace ngfx;
using namespace std;

bool VKShaderModule::initFromFile(VkDevice device,
                                  const std::string &filename) {
  File file;
#ifdef USE_PRECOMPILED_SHADERS
  if (!file.read(filename + ".spv")) {
      return false;
  }
  initFromByteCode(device, file.data.get(), file.size);
#else
  NGFX_ERR("TODO: Support runtime shader compilation");
#endif
  return true;
}
void VKShaderModule::initFromByteCode(VkDevice device, void *data,
                                      uint32_t size) {
  this->device = device;
  VkResult vkResult;
  VkShaderModuleCreateInfo moduleCreateInfo = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, size,
      (const uint32_t *)data};
  V(vkCreateShaderModule(this->device, &moduleCreateInfo, NULL, &v));
}

VKShaderModule::~VKShaderModule() {
  if (v)
    VK_TRACE(vkDestroyShaderModule(device, v, nullptr));
}

template <typename T>
static std::unique_ptr<T> createShaderModule(Device *device,
                                             const std::string &filename) {
  auto vkShaderModule = make_unique<T>();
  if (!vkShaderModule->initFromFile(vk(device)->v, filename)) {
      return nullptr;
  }
  vkShaderModule->initBindings(filename + ".map");
  return vkShaderModule;
}

unique_ptr<VertexShaderModule>
VertexShaderModule::create(Device *device, const std::string &filename) {
  return createShaderModule<VKVertexShaderModule>(device, filename);
}

unique_ptr<FragmentShaderModule>
FragmentShaderModule::create(Device *device, const std::string &filename) {
  return createShaderModule<VKFragmentShaderModule>(device, filename);
}

unique_ptr<ComputeShaderModule>
ComputeShaderModule::create(Device *device, const std::string &filename) {
  return createShaderModule<VKComputeShaderModule>(device, filename);
}
