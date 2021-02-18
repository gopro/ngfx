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
#include "ngfx/MatrixMultiplyApp.h"
#include "ngfx/computeOps/MatrixMultiplyCPUOp.h"
#include "ngfx/computeOps/MatrixMultiplyGPUOp.h"
#include "ngfx/graphics/ShaderModule.h"
#include <glm/gtx/string_cast.hpp>
#include "ngfx/core/DebugUtil.h"
#include <memory>
#define VALIDATE_RESULT
using namespace ngfx;
using namespace glm;
using namespace std;

MatrixMultiplyApp::MatrixMultiplyApp() : ComputeApplication("Matrix Multiply") {}

void MatrixMultiplyApp::onInit() {
    src0.resize(MATRIX_SIZE); src1.resize(MATRIX_SIZE); dst.resize(MATRIX_SIZE);
    for (uint32_t j = 0; j < MATRIX_SIZE; j++) {
        src0[j] = (rand() % 1000)/100.0f; src1[j] = (rand() % 1000)/100.0f;
    }
    matrixMultiplyOp.reset(new MatrixMultiplyGPUOp(
        graphicsContext.get(), 
        { MATRIX_DIM, MATRIX_DIM, src0.data() },
        { MATRIX_DIM, MATRIX_DIM, src1.data() },
        { MATRIX_DIM, MATRIX_DIM, dst.data() }
    ));
}

void MatrixMultiplyApp::onRecordCommandBuffer(CommandBuffer* commandBuffer) {
    graphics->beginComputePass(commandBuffer);
    matrixMultiplyOp->apply(commandBuffer, graphics.get());
    graphics->endComputePass(commandBuffer);
}

void MatrixMultiplyApp::onComputeFinished() {
#ifdef VALIDATE_RESULT
    auto gpuOp = matrixMultiplyOp.get();
    float* dst = (float*)gpuOp->bDst->map();
    //verify against CPU matrix multiply result
    std::vector<float> ref(MATRIX_DIM * MATRIX_DIM, 0);
    MatrixMultiplyCPUOp cpuOp(
        { MATRIX_DIM, MATRIX_DIM,  src0.data() },
        { MATRIX_DIM, MATRIX_DIM, src1.data() },
        { MATRIX_DIM, MATRIX_DIM, ref.data() }
    );
    cpuOp.apply();
    const float ERR_THRESHOLD = 0.02f;
    for (int j = 0; j < ref.size(); j++) {
        if (fabs(ref[j] - dst[j]) > ERR_THRESHOLD) ERR("%d %f %f", j, ref[j], dst[j]);
    }
    gpuOp->bDst->unmap();
#endif
}

int main() {
    MatrixMultiplyApp app;
    app.run();
    return 0;
}
