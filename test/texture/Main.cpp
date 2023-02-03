#include <iostream>
#include "ngfx/drawOps/DrawTextureOp.h"
#include "ngfx/graphics/TextureUtil.h"
#include "test/common/UnitTest.h"
using namespace ngfx;
using namespace std;

class TextureTestOp : public FilterOp {
public:
    TextureTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h)
        : FilterOp(ctx, graphics, w, h) {
        SamplerDesc samplerDesc = {
            FILTER_LINEAR, FILTER_LINEAR, FILTER_NEAREST,
            CLAMP_TO_EDGE, CLAMP_TO_EDGE, CLAMP_TO_EDGE
        };
        texture.reset(TextureUtil::load(ctx, graphics, NGFX_TEST_DATA_DIR "/download.png", IMAGE_USAGE_SAMPLED_BIT, TEXTURE_TYPE_2D, false, 1, &samplerDesc));
        drawTextureOp = make_unique<DrawTextureOp>(ctx, texture.get());
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        drawTextureOp->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawTextureOp> drawTextureOp;
    std::unique_ptr<Texture> texture;
};

int run() {
    UnitTest test("texture");
    test.op = make_unique<TextureTestOp>(test.ctx.get(), test.graphics.get(),
        1386, 1040);
    return test.run();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        return run();
    }
}
