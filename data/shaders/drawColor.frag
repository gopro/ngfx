#version 320 es
precision highp float;
layout (location = 0) out vec4 fragColor;
struct UBOData {
    vec4 color;
};
layout (set = 0, binding = 0, std140) uniform UBO_FS {
    UBOData ubo;
};

void main() {
    fragColor = ubo.color;
}
