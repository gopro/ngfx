#version 320 es
precision highp float;
layout (location = 0) out vec4 fragColor;
const int NUM_COLORS = 16;
struct SSBOData {
	vec4 color[NUM_COLORS];
};

layout (set = 0, binding = 0, std430) buffer SSBO_FS {
    SSBOData ssbo;
};

void main() {
	ivec2 p = ivec2(gl_FragCoord) / ivec2(16);
    fragColor = ssbo.color[4 * (p.y % 4) + p.x % 4];
}
