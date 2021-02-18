#include "common.vert.h"
layout(location = 0) in vec2 pos;

void main() {
	setPos(vec4(pos, 0.0, 1.0));
}
