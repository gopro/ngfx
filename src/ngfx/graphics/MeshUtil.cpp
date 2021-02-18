/*
 * Copyright 2020 GoPro Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "ngfx/graphics/MeshUtil.h"
#include "ngfx/core/DebugUtil.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
using namespace ngfx;
using namespace std;

void MeshUtil::importMesh(const std::string& file, MeshData& meshData) {
	ifstream in(file, ios::binary);
	if (!in.is_open()) NGFX_ERR("cannot open file: %s", file.c_str());
	auto& bounds = meshData.bounds;
	auto& pos = meshData.pos;
	auto& normals = meshData.normal;
	auto& faces = meshData.faces;
	size_t numVerts, numNormals, numFaces;
	in.read((char*)&numVerts, sizeof(numVerts));
	pos.resize(numVerts);
	in.read((char*)value_ptr(bounds[0]), sizeof(bounds[0]));
	in.read((char*)value_ptr(bounds[1]), sizeof(bounds[1]));
	in.read((char*)pos.data(), pos.size() * sizeof(pos[0]));
	in.read((char*)&numNormals, sizeof(numNormals));
	normals.resize(numNormals);
	in.read((char*)normals.data(), normals.size() * sizeof(normals[0]));
	in.read((char*)&numFaces, sizeof(numFaces));
	faces.resize(numFaces);
	in.read((char*)faces.data(), faces.size() * sizeof(faces[0]));
	in.close();
}

void MeshUtil::exportMesh(const std::string& file, MeshData& meshData) {
	ofstream out(file, ios::binary);
	if (!out.is_open()) NGFX_ERR("cannot open file: %s", file.c_str());
	auto& bounds = meshData.bounds;
	auto& pos = meshData.pos;
	auto& normals = meshData.normal;
	auto& faces = meshData.faces;
	size_t numVerts = pos.size(), numNormals = normals.size(), numFaces = faces.size();
	out.write((const char*)&numVerts, sizeof(numVerts));
	out.write((const char*)value_ptr(bounds[0]), sizeof(bounds[0]));
	out.write((const char*)value_ptr(bounds[1]), sizeof(bounds[1]));
	out.write((const char*)pos.data(), pos.size() * sizeof(pos[0]));
	out.write((const char*)&numNormals, sizeof(numNormals));
	out.write((const char*)normals.data(), normals.size() * sizeof(normals[0]));
	out.write((const char*)&numFaces, sizeof(numFaces));
	out.write((const char*)faces.data(), faces.size() * sizeof(faces[0]));
	out.close();
}
