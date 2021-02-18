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
#include "ngfx/computeOps/MatrixMultiplyCPUOp.h"
#include <glm/glm.hpp>
#include "ngfx/core/DebugUtil.h"
#include "ngfx/core/Timer.h"
using namespace ngfx;
using namespace glm;

MatrixMultiplyCPUOp::MatrixMultiplyCPUOp(MatrixParam src0, MatrixParam src1, MatrixParam dst)
    : MatrixMultiplyOp(nullptr), dst(dst) {
    update(src0, src1);
}

MatrixMultiplyCPUOp::~MatrixMultiplyCPUOp() {

}
void MatrixMultiplyCPUOp::apply(CommandBuffer*, Graphics*) {
    matrixMultiply();
}

void MatrixMultiplyCPUOp::update(MatrixParam src0, MatrixParam src1) {
    this->src0 = src0; this->src1 = src1;
    src1t_data.resize(src1.w * src1.h);
    src1t = { src1.h, src1.w, src1t_data.data() };
    transpose(src1, src1t);
}

void MatrixMultiplyCPUOp::transpose(MatrixParam& src, MatrixParam& dst) {
    Timer timer;
    float* dst_data = dst.data;
    for (uint32_t dst_row = 0; dst_row < dst.h; dst_row++) {
        for (uint32_t dst_col = 0; dst_col < dst.w; dst_col++) {
            float* src_data = &src.data[dst_col * src.w + dst_row];
            *dst_data++ = *src_data;
        }
    }
    timer.update();
    NGFX_LOG("transpose elapsed: %f", timer.elapsed);
}

#define VEC4_LOAD(src, j) vec4(src.data[j], src.data[j+1], src.data[j+2], src.data[j+3])

void MatrixMultiplyCPUOp::matrixMultiply() {
    Timer timer;
    float* dst_data = dst.data;
    for (uint32_t dst_row = 0; dst_row < dst.h; dst_row++) {
        uint32_t src0_offset = dst_row * src0.w;
        for (uint32_t dst_col = 0; dst_col < dst.w; dst_col++) {
            uint32_t src1t_offset = dst_col * src1t.h;
            float c = 0.0f;
            for (uint32_t j = 0; j < src0.w; j += 4) {
                vec4 a0 = VEC4_LOAD(src0, src0_offset + j), b0 = VEC4_LOAD(src1t, src1t_offset + j);
                c += dot(a0, b0);
            }
            *dst_data++ = c;
        }
    }
    timer.update();
    NGFX_LOG("CPU matrix multiply elapsed: %f", timer.elapsed);
}

