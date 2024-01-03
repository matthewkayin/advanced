#version 410 core
#begin vertex

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texture_coordinate;

out vec3 frag_pos;
out vec3 normal;
out vec2 texture_coordinate;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(a_pos, 1.0);

    frag_pos = vec3(model * vec4(a_pos, 1.0));
    normal = normalize(mat3(transpose(inverse(model))) * a_normal);
    texture_coordinate = a_texture_coordinate;
}

#begin fragment

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;
};

vec3 calculate_point_light(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_direction);

in vec3 frag_pos;
in vec3 normal;
in vec2 texture_coordinate;

out vec4 frag_color;

uniform vec3 view_pos;
uniform PointLight point_light;
uniform sampler2D model_texture;

void main() {
    vec3 view_direction = normalize(view_pos - frag_pos);
    vec4 light_result = vec4(calculate_point_light(point_light, normal, frag_pos, view_direction), 1.0);
    frag_color = light_result * vec4(1.0, 0.0, 0.0, 1.0);
    // frag_color = light_result * texture(model_texture, texture_coordinate);
}

vec3 calculate_point_light(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_direction) {
    vec3 light_color = vec3(1.0);

    vec3 ambient = 0.5 * light_color;

    vec3 light_direction = normalize(light.position - frag_pos);
    float diffuse_strength = 0.5 * max(dot(normal, light_direction), 0.0); 
    vec3 diffuse = diffuse_strength * light_color;

    vec3 reflect_direction = reflect(-light_direction, normal);
    vec3 specular = vec3(0.0);
    if (diffuse_strength > 0.0) {
        specular = 1.0 * pow(max(dot(view_direction, reflect_direction), 0.0), 32.0) * light_color;
    }

    float vertex_distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + (light.linear * vertex_distance) + (light.quadratic * vertex_distance * vertex_distance));

    return (ambient + diffuse + specular) * attenuation;
}