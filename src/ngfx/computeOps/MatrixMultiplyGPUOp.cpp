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
#include "ngfx/computeOps/MatrixMultiplyGPUOp.h"
#include "ngfx/computeOps/MatrixMultiplyCPUOp.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/core/Timer.h"
#include "ngfx/graphics/BufferUtil.h"
using namespace ngfx;

MatrixMultiplyGPUOp::MatrixMultiplyGPUOp(GraphicsContext *ctx, MatrixParam src0,
                                         MatrixParam src1, MatrixParam dst)
    : MatrixMultiplyOp(ctx), dst(dst) {
  update(src0, src1);
  createPipeline();
}

MatrixMultiplyGPUOp::~MatrixMultiplyGPUOp() {}
void MatrixMultiplyGPUOp::apply(CommandBuffer *commandBuffer,
                                Graphics *graphics) {
  graphics->bindComputePipeline(commandBuffer, computePipeline);
  graphics->bindUniformBuffer(commandBuffer, bUbo.get(), U_UBO,
                              SHADER_STAGE_COMPUTE_BIT);
  graphics->bindStorageBuffer(commandBuffer, bSrc0.get(), SSBO_SRC0,
                              SHADER_STAGE_COMPUTE_BIT);
  graphics->bindStorageBuffer(commandBuffer, bSrc1.get(), SSBO_SRC1,
                              SHADER_STAGE_COMPUTE_BIT);
  graphics->bindStorageBuffer(commandBuffer, bDst.get(), SSBO_DST,
                              SHADER_STAGE_COMPUTE_BIT);
  graphics->dispatch(commandBuffer, dst.w, dst.h, 1, 1, 1, 1);
}

void MatrixMultiplyGPUOp::update(MatrixParam src0, MatrixParam src1) {
  std::vector<float> src1t_data(size_t(src1.w * src1.h));
  MatrixParam src1t = {src1.h, src1.w, src1t_data.data()};
  MatrixMultiplyCPUOp::transpose(src1, src1t);
  UboData uboData = {int32_t(src0.w),  int32_t(src0.h), int32_t(src1t.w),
                     int32_t(src1t.h), int32_t(dst.w),  int32_t(dst.h)};
  bUbo.reset(createUniformBuffer(ctx, &uboData, sizeof(uboData)));
  bSrc0.reset(
      createStorageBuffer(ctx, src0.data, src0.w * src0.h * sizeof(float)));
  bSrc1.reset(
      createStorageBuffer(ctx, src1t.data, src1.w * src1.h * sizeof(float)));
  bDst.reset(createStorageBuffer(ctx, dst.data, dst.w * dst.h * sizeof(float)));
}

void MatrixMultiplyGPUOp::createPipeline() {
  const std::string key = "matrixMultiplyOp";
  computePipeline = (ComputePipeline *)ctx->pipelineCache->get(key);
  if (computePipeline)
    return;
  computePipeline = ComputePipeline::create(
      ctx,
      ComputeShaderModule::create(ctx->device, "matrixMultiply.comp").get());
  ctx->pipelineCache->add(key, computePipeline);
}
