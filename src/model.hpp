#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <map>

struct Mesh {
    GLuint vao;
    GLuint vbo;
    unsigned int vertex_data_size;
};

struct Model {
    std::map<std::string, Mesh> mesh;
};

bool model_init();
bool model_load(Model* model, std::string paths);
bool model_texture_load(GLuint* texture, std::string path);
void model_render(Model& model, glm::vec3 position);