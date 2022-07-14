#version 320 es
precision highp float;
layout (location = 0) in vec4 v_color;
layout (location = 0) out vec4 fragColor;

void main() {
    fragColor = v_color;
}
