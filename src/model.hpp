#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Model {
    GLuint vao;
    GLuint vbo;
    unsigned int vertex_data_size;
};

enum ModelUnit {
    MODEL_UNIT_TANK,
    MODEL_UNIT_COUNT
};

enum ModelUnitColor {
    MODEL_UNIT_COLOR_BLUE,
    MODEL_UNIT_COLOR_RED,
    MODEL_UNIT_COLOR_COUNT
};

struct ModelUnitTransform {
    glm::vec3 position;
    ModelUnitColor color;
};

bool model_init();
bool model_load(Model* model, std::vector<std::string> paths);
bool model_texture_load(GLuint* texture, std::string path);

void model_unit_queue_render(ModelUnit model_unit, ModelUnitTransform transform);
void model_unit_render_from_queues();