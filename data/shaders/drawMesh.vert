#include "common.vert.h"

layout(location = 0) in vec3 inPos;
layout(location = 0) out vec3 outViewPos;
layout(location = 1) in vec3 inNormal;
layout(location = 1) out vec3 outViewNormal;

struct UBO_VS_Data {
    mat4 modelView;
    mat4 modelViewInverseTranspose;
    mat4 modelViewProj;
};
layout (set = 0, binding = 0, std140) uniform UBO_VS {
    UBO_VS_Data ubo;
};

void main() {
	outViewPos = vec3(ubo.modelView * vec4(inPos, 1.0));
	outViewNormal = vec3(ubo.modelViewInverseTranspose * vec4(normalize(inNormal), 0.0));
	setPos(ubo.modelViewProj * vec4(inPos, 1.0));
}
