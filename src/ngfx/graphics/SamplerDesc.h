#pragma once
#include "GraphicsCore.h"

namespace ngfx {
	struct SamplerDesc {
		FilterMode minFilter, magFilter, mipFilter;
		SamplerAddressMode addressModeU, addressModeV, addressModeW;
	};
}