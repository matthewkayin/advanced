#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

struct Model {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint texture;
    unsigned int vertex_data_size;
};

bool model_load(Model* model, std::string path);