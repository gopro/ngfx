#include "ngfx/MeshTool.h"
#include "ngfx/core/DebugUtil.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
using namespace ngfx;
using namespace std;

void MeshTool::updateBounds(MeshData& meshData) {
	auto& b = meshData.bounds;
	for (auto& p : meshData.pos) {
		b[0] = glm::min(p, b[0]);
		b[1] = glm::max(p, b[1]);
	}
}

void MeshTool::importPLY(const std::string& file, MeshData& meshData) {
	ifstream in(file, ios::binary);
	if (!in.is_open()) ERR("cannot open file: %s", file.c_str());
	string param, format; double version;
	in >> param; assert(param == "ply");
	in >> param; assert(param == "format");
	in >> format; assert(format == "binary_little_endian");
	in >> version;
	uint32_t numVerts, numFaces;
	auto skipline = [&]() { in.ignore(1024, '\n'); };
	bool hasNormals = false;
	while (in >> param) {
		if (param == "element") {
			in >> param; 
			if (param == "vertex") in >> numVerts;
			else if (param == "face") in >> numFaces;
		}
		else if (param == "property") {
			in >> param;
			if (param == "list") {
				in >> param; assert(param == "uchar");
				in >> param; assert(param == "int");
				in >> param;
			}
			else { 
				in >> param; 
				if (param == "nx") hasNormals = true;
			}
		}
		else if (param == "end_header") {
			skipline();
			break;
		}
		else {
			skipline();
		}
	}
	assert(hasNormals);
	meshData.pos.resize(numVerts);
	meshData.normal.resize(numVerts);
	for (uint32_t j = 0; j < numVerts; j++) {
		auto& pos = meshData.pos[j];
		auto& normal = meshData.normal[j];
		in.read((char*)value_ptr(pos), sizeof(pos));
		in.read((char*)value_ptr(normal), sizeof(normal));
	}
	meshData.faces.resize(numFaces);
	for (uint32_t j = 0; j < numFaces; j++) {
		auto& face = meshData.faces[j];
		uint8_t numFaceIndices = 0;
		in.read((char*)&numFaceIndices, sizeof(numFaceIndices));
		assert(numFaceIndices == 3);
		in.read((char*)value_ptr(face), sizeof(face));
	}
	in.close();
	updateBounds(meshData);
}