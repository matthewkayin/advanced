#version 410 core
#begin vertex

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texture_coordinate;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(a_pos, 1.0);
}

#begin fragment

out vec4 frag_color;

void main() {
    frag_color = vec4(1.0);
}