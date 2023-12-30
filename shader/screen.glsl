#version 410 core
#begin vertex

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_coordinate;

out vec2 texture_coordinate;

void main() {
    gl_Position = vec4(a_pos.x, a_pos.y, 0.0, 1.0);
    texture_coordinate = a_texture_coordinate;
}

#begin fragment

in vec2 texture_coordinate;

out vec4 color;

uniform sampler2D screen_texture;

void main() {
    color = texture(screen_texture, texture_coordinate);
}