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
#pragma once
#include "ngfx/compute/ComputeOp.h"
#include "ngfx/compute/ComputeUtil.h"

namespace ngfx {
    class ConvolveGPUOp : public ComputeOp {
    public:
        ConvolveGPUOp(GraphicsContext* ctx, Graphics* graphics);
        virtual ~ConvolveGPUOp();
        void apply(CommandBuffer* commandBuffer = nullptr,
            Graphics* graphics = nullptr) override;
        void update(Texture* srcTexture, Texture* dstTexture, ComputeUtil::kernel_t kernel);
        void setKernel(ComputeUtil::kernel_t kernel);
        std::unique_ptr<Buffer> bUbo;
        Texture* srcTexture = nullptr;
        Texture* dstTexture = nullptr;
    protected:
        void createPipeline();
        static const int MAX_KERNEL_SIZE = 64;
        struct ConvolveUboData {
            int kernel_w = 0, kernel_h = 0, padding_0 = 0, padding_1 = 0;
            glm::vec4 kernel_data[MAX_KERNEL_SIZE]{};
        };
        ConvolveUboData uboData;
        ComputePipeline* computePipeline;
        uint32_t U_UBO = 0, U_SRC_IMAGE = 1, U_DST_IMAGE = 2;
    };
}