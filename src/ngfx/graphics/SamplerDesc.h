#pragma once
#include "GraphicsCore.h"

namespace ngfx {
	/** \struct SamplerDesc
	 *  This data structure defines parameters for sampling a texture
	 */
	struct SamplerDesc {
		FilterMode minFilter, magFilter, mipFilter;
		SamplerAddressMode addressModeU, addressModeV, addressModeW;
	};
}