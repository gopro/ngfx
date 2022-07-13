#version 450
#include "common.vert.h"
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 translate;

void main() {
	setPos(vec4(pos + translate, 0.0, 1.0));
}
