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

#include "ngfx/porting/metal/MTLPipelineUtil.h"
using namespace ngfx;

void MTLPipelineUtil::parseDescriptors(
        ShaderModule::DescriptorInfos& descriptorInfos,
        std::vector<uint32_t>& descriptorBindings) {
    uint32_t numBuffers = 0, numTextures = 0;
    for (uint32_t j = 0; j<descriptorInfos.size(); j++) {
        auto& descriptorInfo = descriptorInfos[j];
        if (descriptorInfo.type == DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
            descriptorInfo.type == DESCRIPTOR_TYPE_STORAGE_BUFFER)
            descriptorBindings.push_back(numBuffers++);
        else if (descriptorInfo.type == DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            descriptorBindings.push_back(numTextures++);
    }
}
