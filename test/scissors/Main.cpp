#include <iostream>
#include "test/common/UnitTest.h"
#include "ngfx/drawOps/DrawColorOp.h"
#include "ngfx/graphics/Colors.h"
using namespace ngfx;
using namespace std;
using namespace glm;

class ScissorsTestOp : public FilterOp {
public:
    ScissorsTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h)
        : FilterOp(ctx, graphics, w, h), primitiveTopology(primitiveTopology) {
        vector<vec2> pos = { vec2(-1.0f, 1.0f), vec2(0.0f, -1.0f), vec2(1.0f, 1.0f) };
        op[0] = make_unique<DrawColorOp>(ctx, pos, Color::Red);
        op[1] = make_unique<DrawColorOp>(ctx, pos, Color::Green);
        pos = { vec2(-1.0f, 1.0f), vec2(-1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(0.0f) };
        op[2] = make_unique<DrawColorOp>(ctx, pos, vec4(0.0f, 1.0f, 0.0f, 0.2f),
            [&](GraphicsPipeline::State& state) {
                state.blendEnable = true;
                state.blendParams = BlendUtil::getBlendParams(BlendMode::SRC_OVER);
            });
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        uint32_t w = outputFramebuffer->w, h = outputFramebuffer->h;
        op[0]->draw(commandBuffer, graphics);
        graphics->setScissor(commandBuffer, { 0, 0, w/2, h/2 });
        op[1]->draw(commandBuffer, graphics);
        graphics->setScissor(commandBuffer, { 0, 0, w, h });
        op[2]->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawColorOp> op[3];
    PrimitiveTopology primitiveTopology;
};

int run() {
    UnitTest test("scissors");
    test.op = make_unique<ScissorsTestOp>(test.ctx.get(), test.graphics.get(), test.outputWidth, test.outputHeight);
    return test.run();
}

int main() {
    return run();
}
