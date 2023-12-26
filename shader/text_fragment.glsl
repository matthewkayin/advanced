#version 410 core

in vec2 fragment_vertex_pos;

out vec4 color;

uniform sampler2D u_texture;

void main() {
    color = vec4(vec3(1.0), texture(u_texture, fragment_vertex_pos).r);
    // color = texture(u_texture, fragment_vertex_pos);
}