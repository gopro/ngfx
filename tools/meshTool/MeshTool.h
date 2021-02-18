#pragma once
#include <string>
#include "ngfx/graphics/MeshData.h"

namespace ngfx {
	struct MeshTool {
		static void importPLY(const std::string& file, MeshData& meshData);
		static void updateBounds(MeshData& meshData);
	};
}