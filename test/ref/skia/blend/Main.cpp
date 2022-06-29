#include <include/core/SkBitmap.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkImageEncoder.h>
#include <include/core/SkImage.h>
#include <include/codec/SkCodec.h>
#include <cassert>
#include <map>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

static sk_sp<SkImage> loadImage(fs::path path) {
    auto srcImageData = SkData::MakeFromFileName(path.string().c_str());
    auto srcCodec = SkCodec::MakeFromData(srcImageData);
    return get<0>(srcCodec->getImage());
}

static SkBitmap drawSrc() {
    const int w = 256, h = 256;
    SkBitmap bmp;
    bmp.allocN32Pixels(w, h);
    SkCanvas canvas(bmp);
    SkPaint paint;
    SkColor rectColor = SkColorSetARGB(255, 230, 30, 100);
    paint.setColor(rectColor);
    canvas.drawRect({ 80, 20, 240, 180 }, paint);
    return bmp;
}

static SkBitmap drawDst() {
    const int w = 256, h = 256;
    SkBitmap bmp;
    bmp.allocN32Pixels(w, h);
    SkCanvas canvas(bmp);
    SkPaint paint;
    SkColor rectColor = SkColorSetARGB(255, 30, 150, 240);
    paint.setColor(rectColor);
    canvas.drawRect({ 20, 80, 180, 240 }, paint);
    return bmp;
}

class BlendTest {
public:
    BlendTest() {
        const int w = 256, h = 256;
        bmp = make_unique<SkBitmap>();
        bmp->allocN32Pixels(w, h);
        canvas = make_unique<SkCanvas>(*bmp);
        srcImage = drawSrc().asImage();
        dstImage = drawDst().asImage();
    }
    void run(string testName, SkBlendMode blendMode) {
        SkSamplingOptions samplingOptions;
        SkPaint paint;
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawImage(dstImage.get(), 0.0f, 0.0f, samplingOptions, &paint);
        paint.setBlendMode(blendMode);
        canvas->drawImage(srcImage.get(), 0.0f, 0.0f, samplingOptions, &paint);
        string filename = testName + ".png";
        SkFILEWStream file(filename.c_str());
        SkEncodeImage(&file, *bmp, SkEncodedImageFormat::kPNG, 100);
    }
    unique_ptr<SkBitmap> bmp;
    unique_ptr<SkCanvas> canvas;
    sk_sp<SkImage> srcImage, dstImage;
};

int main(int argc, char** argv) {
    BlendTest test;
    test.run("blend_darken", SkBlendMode::kDarken);
    test.run("blend_lighten", SkBlendMode::kLighten);
    test.run("blend_multiply", SkBlendMode::kMultiply);
    test.run("blend_screen", SkBlendMode::kScreen);
    test.run("blend_overlay", SkBlendMode::kOverlay);

    test.run("blend_src", SkBlendMode::kSrc);
    test.run("blend_src_over", SkBlendMode::kSrcOver);
    test.run("blend_src_in", SkBlendMode::kSrcIn);
    test.run("blend_dst", SkBlendMode::kDst);
    test.run("blend_dst_in", SkBlendMode::kDstIn);
    test.run("blend_clear", SkBlendMode::kClear);
    test.run("blend_src_out", SkBlendMode::kSrcOut);
    test.run("blend_dst_out", SkBlendMode::kDstOut);
    return 0;
}