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

#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include "test/common/UnitTest.h"
#include "ngfx/graphics/BufferUtil.h"
#include "ngfx/drawOps/DrawColorOp.h"
#include "ngfx/graphics/Colors.h"
using namespace std;
using namespace ngfx;
using namespace glm;

enum BufferTestMode { VERTEX, INDEX, UNIFORM, STORAGE, INSTANCING,
    UPLOAD, UPLOAD_SUBREGION, DOWNLOAD, DOWNLOAD_SUBREGION, MAP
};
const vector<string> BufferTestModeStr = { "vertex", "index", "uniform", "storage",
    "instancing", "upload", "upload_subregion", "download", "download_subregion", "map" };

static string toString(BufferTestMode mode) {
    return BufferTestModeStr[mode];
}
static BufferTestMode toBufferTestMode(string v) {
    return BufferTestMode(find(BufferTestModeStr.begin(), BufferTestModeStr.end(), v) - BufferTestModeStr.begin());
}

class VertexBufferTestOp : public FilterOp {
public:
    VertexBufferTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h)
        : FilterOp(ctx, graphics, w, h) {
        int numSteps = 50;
        vector<vec2> pos(numSteps);
        auto it = pos.begin();
        for (int j = 0; j < numSteps; j++) {
            float t = j / float(numSteps - 1);
            float x0 = -1.0f + 2.0f * t;
            float y0 = sin(2 * M_PI * t);
            *it++ = vec2(x0, y0);
        }
        op = make_unique<DrawColorOp>(ctx, pos, vec4(1, 0, 0, 1),
            [&](GraphicsPipeline::State& state) {
                state.primitiveTopology = PRIMITIVE_TOPOLOGY_LINE_STRIP;
        });
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        op->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawColorOp> op;
};
class IndexBufferTestOp : public FilterOp {
public:
    IndexBufferTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h)
        : FilterOp(ctx, graphics, w, h) {
        int numSteps = 20;
        vector<vec2> pos(numSteps + 1);
        auto posIter = pos.begin();
        *posIter++ = vec2(0, 0);
        for (int j = 0; j < numSteps; j++) {
            float t = j / float(numSteps - 1);
            float x0 = -1.0f + 2.0f * t;
            float y0 = sin(M_PI * t);
            *posIter++ = vec2(x0, y0);
        }
        vector<i32> index(2 * (numSteps + 1));
        auto indexIter = index.begin();
        for (int j = 1; j < (numSteps + 1); j++) {
            *indexIter++ = 0;
            *indexIter++ = j;
        }
        op = make_unique<DrawColorOp>(ctx, pos, vec4(1, 0, 0, 1),
            [&](GraphicsPipeline::State& state) {
                state.primitiveTopology = PRIMITIVE_TOPOLOGY_LINE_STRIP;
            }, index);
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        op->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawColorOp> op;
};
class UniformBufferTestOp : public FilterOp {
public:
    UniformBufferTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h)
        : FilterOp(ctx, graphics, w, h) {
        numVerts = 20;
        UBOData uboData = {
            radians(30.0f), radians(300.0f), 0.0f, 0.0f,
            vec4(0,1,0,1),
            0.6f, 0.5f,
            0.2f, -0.1f,
            numVerts
        };
        bUbo.reset(createUniformBuffer(ctx, &uboData, sizeof(uboData)));
        createPipeline();
        graphicsPipeline->getBindings({ &U_UBO }, {});
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) {
        graphics->bindGraphicsPipeline(commandBuffer, graphicsPipeline.get());
        graphics->bindUniformBuffer(commandBuffer, bUbo.get(), U_UBO,
            SHADER_STAGE_VERTEX_BIT);
        graphics->draw(commandBuffer, numVerts);
    }
    struct UBOData {
        float theta0;
        float theta1;
        float padding0, padding1;
        vec4 color;
        float xScale;
        float yScale;
        float xTranslate;
        float yTranslate;
        int numVerts;
    };
    void createPipeline() {
        GraphicsPipeline::State state;
        state.primitiveTopology = PRIMITIVE_TOPOLOGY_LINE_STRIP;
        auto device = ctx->device;
        graphicsPipeline.reset(GraphicsPipeline::create(
            ctx, state,
            VertexShaderModule::create(device, NGFX_TEST_DATA_DIR "/shaders/testUbo.vert").get(),
            FragmentShaderModule::create(device, NGFX_TEST_DATA_DIR "/shaders/testUbo.frag")
            .get(),
            ctx->surfaceFormat, ctx->depthStencilFormat)
        );
    }
    unique_ptr<Buffer> bUbo;
    unique_ptr<GraphicsPipeline> graphicsPipeline;
    uint32_t U_UBO;
    int numVerts;
};
class StorageBufferTestOp : public FilterOp {
public:
    StorageBufferTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h)
        : FilterOp(ctx, graphics, w, h) {
        SSBOData ssboData = {
            Color::Cyan, Color::Red, Color::Blue, Color::Magenta,
            Color::Green, Color::Black, Color::Yellow, Color::Cyan,
            Color::Magenta, Color::Blue, Color::Red, Color::Green,
            Color::Yellow, Color::Cyan, Color::Gray, Color::Red,
        };
        vector<vec2> pos = { vec2(-1, 1), vec2(-1, -1), vec2(1, 1), vec2(1, -1) };
        numVerts = pos.size();
        bPos.reset(createVertexBuffer<vec2>(ctx, pos));
        bSsbo.reset(createStorageBuffer(ctx, &ssboData, sizeof(ssboData)));
        createPipeline();
        graphicsPipeline->getBindings({ &U_SSBO }, { &B_POS });
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) {
        graphics->bindGraphicsPipeline(commandBuffer, graphicsPipeline.get());
        graphics->bindVertexBuffer(commandBuffer, bPos.get(), B_POS, sizeof(vec2));
        graphics->bindStorageBuffer(commandBuffer, bSsbo.get(), U_SSBO,
            SHADER_STAGE_FRAGMENT_BIT, false);
        graphics->draw(commandBuffer, numVerts);
    }
    static const int NUM_COLORS = 16;
    struct SSBOData {
        vec4 colors[NUM_COLORS];
    };
    void createPipeline() {
        GraphicsPipeline::State state;
        state.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        auto device = ctx->device;
        graphicsPipeline.reset(GraphicsPipeline::create(
            ctx, state,
            VertexShaderModule::create(device, NGFX_TEST_DATA_DIR "/shaders/testSSBO.vert").get(),
            FragmentShaderModule::create(device, NGFX_TEST_DATA_DIR "/shaders/testSSBO.frag")
            .get(),
            ctx->surfaceFormat, ctx->depthStencilFormat)
        );
    }
    unique_ptr<Buffer> bPos, bSsbo;
    unique_ptr<GraphicsPipeline> graphicsPipeline;
    uint32_t B_POS, U_SSBO;
    int numVerts;
};
class InstancingTestOp : public FilterOp {

};
class BufferUploadTestOp : public FilterOp {

};
class BufferUploadSubregionTestOp : public FilterOp {

};
class BufferDownloadSubregionTestOp : public FilterOp {

};
class BufferMapTestOp : public FilterOp {

};

#define CASE(p0, p1) \
case p0: \
test.op = make_unique<p1##TestOp>(test.ctx.get(), test.graphics.get(), test.outputWidth, test.outputHeight); \
break

int run(BufferTestMode mode) {
    UnitTest test("buffer_" + toString(mode));
    switch (mode) {
        CASE(VERTEX, VertexBuffer);
        CASE(INDEX, IndexBuffer);
        CASE(UNIFORM, UniformBuffer);
        CASE(STORAGE, StorageBuffer);
        //CASE(INSTANCING, Instancing);
        //CASE(UPLOAD, BufferUpload);
        //CASE(UPLOAD_SUBREGION, BufferUploadSubregion);
        //CASE(DOWNLOAD_SUBREGION, BufferDownloadSubregion);
        //CASE(MAP, BufferMap);
    }
    return test.run();
}

int main(int argc, char** argv) {
	if (argc < 2) {
        vector<BufferTestMode> testModes = {
            VERTEX, INDEX, UNIFORM, STORAGE, /*INSTANCING,
            UPLOAD, UPLOAD_SUBREGION, DOWNLOAD, DOWNLOAD_SUBREGION, MAP*/
        };
        int r = 0;
        for (BufferTestMode m : testModes)
            r |= run(m);
        return r;
	}
    string bufferTestModeStr = argv[1];
    return run(toBufferTestMode(bufferTestModeStr));
	return 0;
}
