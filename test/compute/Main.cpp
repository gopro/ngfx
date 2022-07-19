#include <iostream>
#include "ngfx/graphics/GraphicsContext.h"
#include "ngfx/computeOps/MatrixMultiplyGPUOp.h"
#include "ngfx/computeOps/MatrixMultiplyCPUOp.h"
#include "test/common/UnitTest.h"
#include <array>
using namespace ngfx;
using namespace std;

int compareData(float* v0, float* v1, int size) {
    for (int j = 0; j < size; j++) {
        if (abs(v0[j] - v1[j]) > FLT_EPSILON)
            return 1;
    }
    return 0;
}

int run() {
    unique_ptr<GraphicsContext> ctx;
    ctx.reset(GraphicsContext::create("compute_matrix_multiply", false));
    ctx->setSurface(nullptr);
    unique_ptr<Graphics> graphics;
    graphics.reset(Graphics::create(ctx.get()));
    using MatrixParam = MatrixMultiplyOp::MatrixParam;
    const int DIM = 16; // 4096;
    vector<float> srcData[2], dstData[2];
    for (int j = 0; j < 2; j++) {
        srcData[j] = vector<float>(DIM * DIM);
        dstData[j] = vector<float>(DIM * DIM);
    }
    for (int j = 0; j < (DIM * DIM); j++) {
        srcData[0][j] = (rand() % 1024) / 1024.0f;
        srcData[1][j] = (rand() % 1024) / 1024.0f;
    }
    MatrixParam src0 = { DIM, DIM, srcData[0].data() };
    MatrixParam src1 = { DIM, DIM, srcData[1].data() };
    MatrixParam dst0 = { DIM, DIM, dstData[0].data() };
    //test matrix multiply on GPU
    auto op = make_unique<MatrixMultiplyGPUOp>(ctx.get(),
        src0, src1, dst0);
    auto commandBuffer = ctx->computeCommandBuffer();
    commandBuffer->begin();
    op->apply(commandBuffer, graphics.get());
    commandBuffer->end();
    ctx->submit(commandBuffer);
    ctx->queue->waitIdle();
    //compare against CPU
    MatrixParam dst1 = { DIM, DIM, dstData[1].data() };
    auto cpuOp = make_unique<MatrixMultiplyCPUOp>(src0, src1, dst1);
    cpuOp->apply();
    //compare results
    return compareData(dstData[0].data(), dstData[1].data(), DIM * DIM);
}

int main(int argc, char** argv) {
    return run();
}
