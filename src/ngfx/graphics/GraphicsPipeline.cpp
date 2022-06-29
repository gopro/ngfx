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
#include "ngfx/graphics/GraphicsPipeline.h"
#include "ngfx/core/HashUtil.h"
#include <set>
using namespace ngfx;

void GraphicsPipeline::getBindings(
    std::vector<uint32_t *> pDescriptorBindings,
    std::vector<uint32_t *> pVertexAttribBindings) {
  for (uint32_t j = 0; j < pDescriptorBindings.size(); j++)
    *pDescriptorBindings[j] = descriptorBindings[j];
  for (uint32_t j = 0; j < pVertexAttribBindings.size(); j++)
    *pVertexAttribBindings[j] = vertexAttributeBindings[j];
}


size_t BlendParams::key() {
    return HashUtil::combine(srcColorBlendFactor, dstColorBlendFactor, srcAlphaBlendFactor, dstAlphaBlendFactor,
        colorBlendOp, alphaBlendOp);
}

size_t StencilParams::key() {
    return HashUtil::combine(
        stencilReadMask,
        stencilWriteMask,
        frontStencilFailOp,
        frontStencilDepthFailOp,
        frontStencilPassOp,
        frontStencilFunc,
        backStencilFailOp,
        backStencilDepthFailOp,
        backStencilPassOp,
        backStencilFunc,
        stencilRef
    );
}

size_t GraphicsPipeline::State::key() {
    return HashUtil::combine(
        primitiveTopology,
        polygonMode,
        blendEnable,
        blendParams.key(),
        colorWriteMask,
        cullModeFlags,
        frontFace,
        lineWidth,
        depthTestEnable,
        depthWriteEnable,
        stencilEnable,
        depthFunc,
        stencilParams.key(),
        numSamples,
        numColorAttachments
    );
}