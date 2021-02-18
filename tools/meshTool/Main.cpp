#include "ngfx/MeshTool.h"
#include "ngfx/graphics/MeshUtil.h"
#include "ngfx/core/DebugUtil.h"
using namespace ngfx;

int main(int argc, char** argv) {
	if (argc != 3) ERR("usage: ./MeshUtil <input> <output>");
	MeshData meshData;
	MeshTool::importPLY(argv[1], meshData);
	MeshUtil::exportMesh(argv[2], meshData);
}