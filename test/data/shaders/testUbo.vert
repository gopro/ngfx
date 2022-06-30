#version 450
#include "common.vert.h"
layout(location = 0) in vec2 pos;
layout (location = 0) out vec4 v_color;

struct UBOData {
    float theta0;
    float theta1;
    vec4 color;
    float xScale;
    float yScale;
    float xTranslate;
    float yTranslate;
    int numVerts;
};
layout (set = 0, binding = 0, std140) uniform UBO_VS {
    UBOData ubo;
};
void main() {
	int numVerts = ubo.numVerts;
	int vertexIndex = gl_VertexIndex;
	vec2 p0;
	if (vertexIndex == 0 || vertexIndex == (numVerts - 1))
		p0 = vec2(0.0f);
	else {
		float theta = mix(ubo.theta0, ubo.theta1, float(vertexIndex - 1) / float(numVerts - 1 - 1));
		p0 = vec2(cos(theta), sin(theta));
	}
	p0 = vec2(ubo.xTranslate, ubo.yTranslate) * vec2(ubo.xScale, ubo.yScale) * p0;
	setPos(vec4(p0, 0.0, 1.0));
	v_color = ubo.color;
}
