#include "UnitTest.h"
#include "ngfx/graphics/ImageUtil.h"
#include "ngfx/graphics/TextureUtil.h"
#include "ngfx/core/FileUtil.h"
using namespace ngfx;
using namespace std;
namespace fs = std::filesystem;

UnitTest::UnitTest(std::string testName, int outputWidth, int outputHeight, bool enableDepthStencil, 
	GraphicsContext::OnSelectDepthStencilFormats onSelectDepthStencilFormats) :
		testName(testName), outputWidth(outputWidth), outputHeight(outputHeight) {
	ctx.reset(GraphicsContext::create(testName.c_str(), enableDepthStencil, true, onSelectDepthStencilFormats));
	surface.reset(new Surface(outputWidth, outputHeight, true));
	graphics.reset(Graphics::create(ctx.get()));
	ctx->setSurface(surface.get());
	genRefs = getenv("GEN_REFS") ? true : false;
}
int UnitTest::run() {
	auto commandBuffer = ctx->drawCommandBuffer();
	commandBuffer->begin();
	op->apply(ctx.get(), commandBuffer, graphics.get());
	commandBuffer->end();
	ctx->submit(commandBuffer);
	graphics->waitIdle(commandBuffer);
	fs::path refFilePath = NGFX_TEST_DATA_REF_DIR "/" + testName + ".png";
	if (genRefs) {
		TextureUtil::storePNG(refFilePath.string(), op->outputTexture);
		return 0;
	}
	fs::path filePath = "out/" + testName + ".png";
	TextureUtil::storePNG(filePath.string(), op->outputTexture);
	ImageData textureData;
	TextureUtil::download(op->outputTexture, textureData);
	ImageData refImageData;
	ImageUtil::load(refFilePath.string(), refImageData);
	if (textureData.size != refImageData.size) {
		return 1;
	}
	return memcmp(textureData.data, refImageData.data, refImageData.size);
}
