#version 320 es
precision highp float;
layout (set = 0, binding = 0) uniform sampler2D sampler0;
layout (location = 0) in vec2 v_texCoord;
layout (location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(sampler0, v_texCoord);
}
