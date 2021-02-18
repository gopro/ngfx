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

#include "ngfx/porting/metal/MTLComputePipeline.h"
#include "ngfx/porting/metal/MTLComputeCommandEncoder.h"
#include "ngfx/porting/metal/MTLCommandBuffer.h"
#include "ngfx/porting/metal/MTLPipelineUtil.h"
#include "ngfx/porting/metal/MTLShaderModule.h"
#include "ngfx/porting/metal/MTLGraphicsContext.h"
using namespace ngfx;

void MTLComputePipeline::create(ngfx::MTLGraphicsContext *ctx, id<MTLFunction> computeFunction) { 
    NSError* error;
    auto device = ctx->mtlDevice.v;
    mtlPipelineState = [device newComputePipelineStateWithFunction:computeFunction error:&error];
    NSCAssert(mtlPipelineState, @"Failed to create pipeline state: %@", error);
}

ComputePipeline* ComputePipeline::create(GraphicsContext* graphicsContext,
         ComputeShaderModule* cs) {
    MTLComputePipeline* mtlComputePipeline = new MTLComputePipeline();

    auto& descriptorBindings = mtlComputePipeline->descriptorBindings;
    uint32_t numDescriptors =
        cs->descriptors.empty() ? 0 : cs->descriptors.back().set + 1;
    descriptorBindings.resize(numDescriptors);
    MTLPipelineUtil::parseDescriptors(cs->descriptors, descriptorBindings);
        
    mtlComputePipeline->create(mtl(graphicsContext), mtl(cs)->mtlFunction);
    return mtlComputePipeline;
}
