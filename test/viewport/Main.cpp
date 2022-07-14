#include <iostream>
#include "test/common/UnitTest.h"
#include "ngfx/drawOps/DrawColorOp.h"
#include "ngfx/graphics/Colors.h"
using namespace ngfx;
using namespace std;
using namespace glm;

class ViewportTestOp : public FilterOp {
public:
    ViewportTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h)
        : FilterOp(ctx, graphics, w, h), primitiveTopology(primitiveTopology) {
        vector<vec2> pos = { vec2(-1.0f, 1.0f), vec2(-1.0f, -1.0f), vec2(1.0f, 1.0f), vec2(1.0f, -1.0f) };
        op = make_unique<DrawColorOp>(ctx, pos, vec4(1.0f, 0.0f, 0.0f, 0.5f), [&](GraphicsPipeline::State& state) {
            state.setBlendMode(BlendMode::SRC_OVER);
        });
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        int w = outputFramebuffer->w, h = outputFramebuffer->h;
        op->draw(commandBuffer, graphics);
        graphics->setViewport(commandBuffer, { 50, 20, 100, 80 });
        op->draw(commandBuffer, graphics);
        graphics->setViewport(commandBuffer, { w - 50, h - 100, 20, 50 });
        op->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawColorOp> op;
    PrimitiveTopology primitiveTopology;
};

int run() {
    UnitTest test("Viewport");
    test.op = make_unique<ViewportTestOp>(test.ctx.get(), test.graphics.get(), test.outputWidth, test.outputHeight);
    return test.run();
}

int main() {
    return run();
}
