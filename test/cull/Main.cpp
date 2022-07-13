#include <iostream>
#include <vector>
#include "test/common/UnitTest.h"
#include "ngfx/drawOps/DrawColorOp.h"
using namespace std;
using namespace ngfx;
using namespace glm;

static string toString(CullModeFlags mode) {
    switch (mode) {
    case CULL_MODE_NONE: return "none"; break;
    case CULL_MODE_FRONT_BIT: return "front"; break;
    case CULL_MODE_BACK_BIT: return "back"; break;
    default: return ""; break;
    }
}
static CullModeFlags toCullModeFlags(string v) {
    if (v == "none")
        return CULL_MODE_NONE;
    else if (v == "front")
        return CULL_MODE_FRONT_BIT;
    else
        return CULL_MODE_BACK_BIT;
}

class CullTestOp : public FilterOp {
public:
    CullTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h, CullModeFlags cullMode)
        : FilterOp(ctx, graphics, w, h), cullMode(cullMode) {
        vector<vec2> pos = {
            vec2(0,0), vec2(1,0), vec2(1,1),
            vec2(0,0), vec2(-1,0), vec2(-1,1)
        };
        
        op = make_unique<DrawColorOp>(ctx, pos, vec4(1, 0, 0, 1),
            [&](GraphicsPipeline::State& state) {
                state.cullModeFlags = cullMode;
                state.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            });
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        op->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawColorOp> op;
    CullModeFlags cullMode;
};

int run(CullModeFlags mode) {
    UnitTest test("cull_" + toString(mode));
    test.op = make_unique<CullTestOp>(test.ctx.get(), test.graphics.get(), test.outputWidth, test.outputHeight, mode);
    return test.run();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        vector<CullModeFlags> testModes = {
            CULL_MODE_NONE, CULL_MODE_FRONT_BIT, CULL_MODE_BACK_BIT
        };
        int r = 0;
        for (CullModeFlags m : testModes)
            r |= run(m);
        return r;
    }
    string cullTestModeStr = argv[1];
    return run(toCullModeFlags(cullTestModeStr));
    return 0;
}
