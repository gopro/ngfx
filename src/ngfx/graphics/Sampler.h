#pragma once
#include "ngfx/graphics/SamplerDesc.h"

namespace ngfx {
	class GraphicsContext;
	/** \class Sampler
	 *  This class defines a sampler object for sampling a texture.
	 */
	class Sampler {
	public:
		static Sampler* create(GraphicsContext* ctx, const SamplerDesc& samplerDesc);
		virtual ~Sampler() {}
	};
}