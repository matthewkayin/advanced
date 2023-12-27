#version 410 core
#begin vertex

layout (location = 0) in vec2 vertex_position;

out vec2 fragment_vertex_pos;

uniform vec2 screen_size;
uniform vec2 render_coords;
uniform vec2 render_size;

void main() {
    vec2 render_position = vec2(render_coords.x + (vertex_position.x * render_size.x), render_coords.y + (vertex_position.y * render_size.y));
    render_position = vec2((render_position.x / (screen_size.x / 2)) - 1, 1 - (render_position.y / (screen_size.y / 2)));
    gl_Position = vec4(render_position, 0.0, 1.0);
    fragment_vertex_pos = vertex_position;
}

#begin fragment

in vec2 fragment_vertex_pos;

out vec4 color;

uniform sampler2D u_texture;
uniform vec2 texture_offset;
uniform vec2 render_size;
uniform vec2 atlas_size;
uniform vec3 font_color;

void main() {
    vec2 texture_coordinate = vec2((texture_offset.x + (render_size.x * fragment_vertex_pos.x)) / atlas_size.x, (texture_offset.y + (render_size.y * fragment_vertex_pos.y)) / atlas_size.y);
    color = vec4(font_color, texture(u_texture, texture_coordinate).r);
}