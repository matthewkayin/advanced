#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <map>

struct Material {
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    GLuint map_ka;
    GLuint map_kd;
};

struct Mesh {
    GLuint vao;
    GLuint vbo;
    unsigned int vertex_data_size;
    std::string material;
};

struct Model {
    std::map<std::string, Mesh> mesh;
    std::map<std::string, Material> material;
};

bool model_init();
bool model_load(Model* model, std::string paths);
bool model_texture_load(GLuint* texture, std::string path);
void model_render(Model& model, glm::vec3 position);