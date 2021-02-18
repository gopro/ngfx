#version 320 es
precision highp float;
struct UBOData { uint level; };
layout (set = 0, binding = 0, std140) uniform UBO_FS {
    UBOData ubo;
};
layout (set = 1, binding = 0) uniform sampler2D sampler0;
layout (location = 0) in vec2 v_texCoord;
layout (location = 0) out vec4 fragColor;

void main() {
    fragColor = textureLod(sampler0, v_texCoord, float(ubo.level));
}
