#version 450
#include "common.vert.h"
layout(location = 0) in vec2 pos;
layout (location = 0) out vec4 v_color;

struct UBOData {
    mat4 transform;
    vec4 color;
};
layout (set = 0, binding = 0, std140) uniform UBO_VS {
    UBOData ubo;
};

void main() {
	setPos(ubo.transform * vec4(pos, 0.0f, 1.0f));
	v_color = ubo.color;
}
