#include <iostream>
#include <vector>
#include "ngfx/graphics/GraphicsCore.h"
#include "ngfx/graphics/FilterOp.h"
#include "ngfx/drawOps/DrawMeshOp.h"
#include "test/common/UnitTest.h"
using namespace ngfx;
using namespace std;

static string toString(PixelFormat v) {
    static const map<PixelFormat, string> formatMap = {
        { PIXELFORMAT_D32_SFLOAT_S8_UINT, "d32_s8" },
        { PIXELFORMAT_D32_SFLOAT, "d32" },
        { PIXELFORMAT_D24_UNORM_S8_UINT, "d24_s8" },
        { PIXELFORMAT_D16_UNORM, "d16" }
    };
    return formatMap.at(v);
}

static PixelFormat toPixelFormat(const string& s) {
    static const map<string, PixelFormat> formatMap = {
        { "d32_s8", PIXELFORMAT_D32_SFLOAT_S8_UINT },
        { "d32", PIXELFORMAT_D32_SFLOAT },
        { "d24_s8", PIXELFORMAT_D24_UNORM_S8_UINT },
        { "d16", PIXELFORMAT_D16_UNORM }
    };
    return formatMap.at(s);
}

class DepthModeTestOp : public FilterOp {
public:
    DepthModeTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h, PixelFormat depthMode)
        : FilterOp(ctx, graphics, w, h), depthMode(depthMode) {
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        drawMeshOp->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawMeshOp> drawMeshOp;
    PixelFormat depthMode;
};

int run(PixelFormat depthFormat) {
    UnitTest test("depth_" + toString(depthFormat));
    test.op = make_unique<DepthModeTestOp>(test.ctx.get(), test.graphics.get(),
        test.outputWidth, test.outputHeight, depthFormat);
    return test.run();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        vector<PixelFormat> depthModes = {
            PIXELFORMAT_D32_SFLOAT_S8_UINT, PIXELFORMAT_D32_SFLOAT,
            PIXELFORMAT_D24_UNORM_S8_UINT, PIXELFORMAT_D16_UNORM,
        };
        int r = 0;
        for (PixelFormat m : depthModes)
            r |= run(m);
        return r;
    }
    string depthModeStr = argv[1];
    return run(toPixelFormat(depthModeStr));
}