#pragma once
#include "ngfx/graphics/GraphicsContext.h"
#include "ngfx/graphics/FilterOp.h"
#include "test/common/UnitTest.h"

namespace ngfx {
	class UnitTest {
	public:
		UnitTest(std::string testName, int outputWidth = 256, int outputHeight = 256);
		int run();
		std::unique_ptr<GraphicsContext> ctx;
		std::unique_ptr<Surface> surface;
		std::unique_ptr<Graphics> graphics;
		std::unique_ptr<FilterOp> op;
		std::string testName;
		int outputWidth = 0, outputHeight = 0;
		bool genRefs = false;
	};
}