#include "common.vert.h"

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 0) out vec2 v_texCoord;

void main() {
	setPos(vec4(pos, 0.0, 1.0));
    v_texCoord = texCoord;
}
