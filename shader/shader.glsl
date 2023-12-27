#version 410 core
#begin vertex

layout (location = 0) in vec3 a_pos;

void main() {
    gl_Position = vec4(a_pos, 1.0);
}

#begin fragment

out vec4 frag_color;

void main() {
    frag_color = vec4(1.0);
}