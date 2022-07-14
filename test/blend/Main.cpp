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

#include <iostream>
#include "ngfx/graphics/GraphicsContext.h"
#include "ngfx/graphics/BufferUtil.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/graphics/TextureUtil.h"
#include "ngfx/graphics/BufferUtil.h"
#include "test/common/UnitTest.h"
#include "ngfx/drawOps/DrawColorOp.h"
#include "ngfx/drawOps/DrawTextureOp.h"
#include <glm/glm.hpp>
#include <filesystem>
using namespace ngfx;
using namespace std;
using namespace glm;

class DrawInputTextureOp : public FilterOp {
public:
    DrawInputTextureOp(GraphicsContext* ctx, Graphics* graphics, vec2 p0, vec2 p1, vec4 rectColor, int w = 256, int h = 256) 
        : FilterOp(ctx, graphics, w, h)  {
        p0 = p0 / vec2(w / 2.0f, h / 2.0f) - vec2(1.0f); p0.y *= -1.0f;
        p1 = p1 / vec2(w / 2.0f, h / 2.0f) - vec2(1.0f); p1.y *= -1.0f;
        vector<vec2> pos = { p0, vec2(p0.x, p1.y), vec2(p1.x, p0.y), p1 };
        rectColor /= 255.0f;
        drawColorOp = make_unique<DrawColorOp>(ctx, pos, rectColor);
    }
    void draw(CommandBuffer *commandBuffer, Graphics *graphics) {
        drawColorOp->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawColorOp> drawColorOp;
};

class BlendTestOp : public FilterOp {
public:
    BlendTestOp(GraphicsContext *ctx, Graphics *graphics, int w, int h, BlendMode blendMode)
        : FilterOp(ctx, graphics, w, h), blendMode(blendMode) {
        drawSrcTextureOp = make_unique<DrawInputTextureOp>(ctx, graphics, vec2(20, 80), vec2(180, 240), vec4(20, 160, 240, 255));
        drawDstTextureOp = make_unique<DrawInputTextureOp>(ctx, graphics, vec2(80, 20), vec2(240, 180), vec4(240, 20, 160, 255));
        drawTextureOp = make_unique<DrawTextureOp>(ctx, drawDstTextureOp->outputTexture.get());
        drawTextureBlendOp = make_unique<DrawTextureOp>(ctx, drawSrcTextureOp->outputTexture.get(),
            [&](GraphicsPipeline::State& state) {
                state.setBlendMode(blendMode);
        });

        auto commandBuffer = ctx->drawCommandBuffer();
        commandBuffer->begin();
        drawSrcTextureOp->apply(ctx, commandBuffer, graphics);
        drawDstTextureOp->apply(ctx, commandBuffer, graphics);
        commandBuffer->end();
        ctx->queue->submit(commandBuffer);
        ctx->queue->waitIdle();
    }
    void draw(CommandBuffer *commandBuffer, Graphics *graphics) override {
        drawTextureOp->draw(commandBuffer, graphics);
        drawTextureBlendOp->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawInputTextureOp> drawSrcTextureOp, drawDstTextureOp;
    std::unique_ptr<DrawTextureOp> drawTextureOp;
    std::unique_ptr<DrawTextureOp> drawTextureBlendOp;
    BlendMode blendMode;
};

int run(BlendMode blendMode) {
    UnitTest test("blend_" + BlendUtil::toString(blendMode));
    test.op = make_unique<BlendTestOp>(test.ctx.get(), test.graphics.get(),
        test.outputWidth, test.outputHeight, blendMode);
    return test.run();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        vector<BlendMode> blendModes = {
            DARKEN, LIGHTEN, MULTIPLY, SCREEN, OVERLAY,
            SRC, SRC_OVER, SRC_IN, DST, DST_IN, CLEAR, SRC_OUT, DST_OUT, DST_OVER
        };
        int r = 0;
        for (BlendMode m : blendModes)
            r |= run(m);
        return r;
    }
    string blendModeStr = argv[1];
    return run(BlendUtil::toBlendMode(blendModeStr));
}

