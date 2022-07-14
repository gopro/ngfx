#include <iostream>
#include <vector>
#include "test/common/UnitTest.h"
#include "ngfx/drawOps/DrawColorOp.h"
#include "glm/gtx/transform.hpp"
#include "ngfx/graphics/Colors.h"
using namespace std;
using namespace ngfx;
using namespace glm;

enum TransformTestMode {
    TRANSLATE, ROTATE, SCALE, COMPOUND
};
const vector<string> TransformTestModeStr = { "translate", "rotate", "scale", "compound" };

static string toString(TransformTestMode mode) {
    return TransformTestModeStr[mode];
}
static TransformTestMode toTransformTestMode(string v) {
    return TransformTestMode(find(TransformTestModeStr.begin(), TransformTestModeStr.end(), v) - TransformTestModeStr.begin());
}

class TransformTestOp : public FilterOp {
public:
    TransformTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h, mat4 transform)
        : FilterOp(ctx, graphics, w, h) {
        UBOData uboData = {
            transform,
            Color::Red,
        };
        vector<vec2> pos = { vec2(-.1f, .1f), vec2(-.1f, -.1f), vec2(.1f), vec2(.1f, -.1f) };
        numVerts = pos.size();
        bPos.reset(createVertexBuffer<vec2>(ctx, pos));
        bUbo.reset(createUniformBuffer(ctx, &uboData, sizeof(uboData)));
        createPipeline();
        graphicsPipeline->getBindings({ &U_UBO }, { &B_POS });
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) {
        graphics->bindGraphicsPipeline(commandBuffer, graphicsPipeline.get());
        graphics->bindVertexBuffer(commandBuffer, bPos.get(), B_POS, sizeof(vec2));
        graphics->bindUniformBuffer(commandBuffer, bUbo.get(), U_UBO,
            SHADER_STAGE_VERTEX_BIT);
        graphics->draw(commandBuffer, numVerts);
    }
    struct UBOData {
        mat4 transform;
        vec4 color;
    };
    void createPipeline() {
        GraphicsPipeline::State state;
        state.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        state.cullModeFlags = CULL_MODE_NONE;
        auto device = ctx->device;
        graphicsPipeline.reset(GraphicsPipeline::create(
            ctx, state,
            VertexShaderModule::create(device, NGFX_TEST_DATA_DIR "/shaders/testTransform.vert").get(),
            FragmentShaderModule::create(device, NGFX_TEST_DATA_DIR "/shaders/testTransform.frag")
            .get(),
            ctx->surfaceFormat, ctx->depthStencilFormat)
        );
    }
    unique_ptr<Buffer> bPos, bUbo;
    unique_ptr<GraphicsPipeline> graphicsPipeline;
    uint32_t B_POS, U_UBO;
    int numVerts;
};

#define FN0(p0, p1) case p0: transform = p1; break

int run(TransformTestMode mode) {
    UnitTest test("transform_" + toString(mode));
    mat4 transform;
    switch (mode) {
        FN0(TRANSLATE, translate(vec3(0.2f, 0.5f, 0.0f)));
        FN0(ROTATE, rotate(radians(60.0f), vec3(0, 0, 1)));
        FN0(SCALE, scale(vec3(2.0f, 0.5f, 1.0f)));
        FN0(COMPOUND, rotate(radians(45.0f), vec3(0, 0, 1)) * scale(vec3(5.0f, 1.0f, 1.0f)) * translate(vec3(0.1f, 0.0f, 0.0f)));
    default: break;
    }
    test.op = make_unique<TransformTestOp>(test.ctx.get(), test.graphics.get(), test.outputWidth, test.outputHeight,
        transform);
    return test.run();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        vector<TransformTestMode> testModes = {
            TRANSLATE, ROTATE, SCALE, COMPOUND
        };
        int r = 0;
        for (TransformTestMode m : testModes)
            r |= run(m);
        return r;
    }
    string transformModeStr = argv[1];
    return run(toTransformTestMode(transformModeStr));
    return 0;
}
