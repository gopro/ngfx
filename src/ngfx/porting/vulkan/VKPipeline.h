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
#include "ngfx/porting/vulkan/VKShaderModule.h"

namespace ngfx {
class VKPipeline {
public:
  virtual ~VKPipeline();
  struct Descriptor {
    VkDescriptorType type;
    VkShaderStageFlags stageFlags = 0;
  };
  struct ShaderStage {
    VkShaderStageFlagBits stage;
    VKShaderModule *module;
  };
  VkPipeline v = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

protected:
  VkDevice device;
};

struct VKPipelineUtil {
  static void
  parseDescriptors(std::vector<ShaderModule::DescriptorInfo> &descriptors,
                   VkShaderStageFlagBits shaderStage,
                   std::vector<VKPipeline::Descriptor> &vkDescriptors,
                   std::vector<uint32_t> &descriptorBindings);
};
}; // namespace ngfx
