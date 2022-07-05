#version 450
#include "common.vert.h"
layout(location = 0) in vec2 pos;

void main() {
	int vertexIndex = gl_VertexIndex;
	setPos(vec4(pos, 0.0, 1.0));
}
