/*
 * Copyright 2022 GoPro Inc.
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
#include "ngfx/computeOps/ConvolveGPUOp.h"
#include "ngfx/graphics/BufferUtil.h"
using namespace ngfx;
using namespace ngfx::ComputeUtil;
using namespace glm;

ConvolveGPUOp::ConvolveGPUOp(GraphicsContext* ctx, Graphics* graphics)
    : ComputeOp(ctx) {
    createPipeline();
    computePipeline->getBindings({ &U_UBO, &U_SRC_IMAGE, &U_DST_IMAGE });
}
ConvolveGPUOp::~ConvolveGPUOp() {}
void ConvolveGPUOp::apply(CommandBuffer* commandBuffer,
    Graphics* graphics) {
    graphics->beginComputePass(commandBuffer);
    graphics->bindComputePipeline(commandBuffer, computePipeline);
    graphics->bindUniformBuffer(commandBuffer, bUbo.get(), U_UBO, SHADER_STAGE_COMPUTE_BIT);
    graphics->bindTextureAsImage(commandBuffer, srcTexture, U_SRC_IMAGE);
    graphics->bindTextureAsImage(commandBuffer, dstTexture, U_DST_IMAGE);
    graphics->dispatch(commandBuffer, dstTexture->w, dstTexture->h, 1, 1, 1, 1);
    graphics->endComputePass(commandBuffer);
}
void ConvolveGPUOp::update(Texture* srcTexture, Texture* dstTexture, kernel_t kernel) {
    this->srcTexture = srcTexture;
    this->dstTexture = dstTexture;
    setKernel(kernel);
}
void ConvolveGPUOp::setKernel(kernel_t kernel) {
    vec4* kernelData = uboData.kernel_data;
    uboData.kernel_w = kernel.w;
    uboData.kernel_h = kernel.h;
    for (int j = 0; j < (kernel.w * kernel.h); j++)
        uboData.kernel_data[j] = vec4(kernel.data[j]);
    bUbo.reset(createUniformBuffer(ctx, &uboData, sizeof(uboData)));
}

void ConvolveGPUOp::createPipeline() {
    computePipeline = ComputePipeline::create(
        ctx,
        ComputeShaderModule::create(ctx->device, NGFX_DATA_DIR "/convolve.comp").get());
}
