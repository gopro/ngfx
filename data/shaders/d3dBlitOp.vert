#version 450
out gl_PerVertex {
    vec4 gl_Position;   
};

void setPos(vec4 pos) {
    gl_Position = vec4(pos.x, pos.y, 0.5f * ( pos.z + pos.w ), pos.w);
}

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 0) out vec2 v_texCoord;

void main() {
    setPos(vec4(pos, 0.0, 1.0));
    v_texCoord = texCoord;
}
