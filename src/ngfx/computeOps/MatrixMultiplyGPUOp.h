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
#include "ngfx/computeOps/MatrixMultiplyOp.h"
#include "ngfx/graphics/Graphics.h"

namespace ngfx {
    class MatrixMultiplyGPUOp : public MatrixMultiplyOp {
    public:
        MatrixMultiplyGPUOp(
            GraphicsContext* ctx,
            MatrixParam src0, MatrixParam src1, MatrixParam dst);
        virtual ~MatrixMultiplyGPUOp();
        virtual void apply(CommandBuffer* commandBuffer = nullptr, Graphics* graphics = nullptr);
        virtual void update(MatrixParam src0, MatrixParam src1);
        std::unique_ptr<Buffer> bUbo;
        std::unique_ptr<Buffer> bSrc0, bSrc1, bDst;
    protected:
        struct UboData { int32_t src0_w, src0_h, src1t_w, src1t_h, dst_w, dst_h; };
        void createPipeline();
        ComputePipeline* computePipeline;
        uint32_t U_UBO = 0, SSBO_SRC0 = 1, SSBO_SRC1 = 2, SSBO_DST = 3; 
        MatrixParam dst;
    };
}
