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
    MODEL_UNIT_INFANTERY,
    MODEL_UNIT_BAZOOKA,
    MODEL_UNIT_COUNT
};

enum ModelUnitColor {
    MODEL_UNIT_COLOR_BLUE,
    MODEL_UNIT_COLOR_RED,
    MODEL_UNIT_COLOR_COUNT
};

enum ModelTerrain {
    MODEL_TERRAIN_AIRPORT,
    MODEL_TERRAIN_BASE,
    MODEL_TERRAIN_BEACH,
    MODEL_TERRAIN_BEACH_INNER_CORNER,
    MODEL_TERRAIN_BEACH_OUTER_CORNER,
    MODEL_TERRAIN_BRIDGE,
    MODEL_TERRAIN_CITY,
    MODEL_TERRAIN_FOREST,
    MODEL_TERRAIN_MOUNTAIN_HIGH,
    MODEL_TERRAIN_MOUNTAIN_LOW,
    MODEL_TERRAIN_QG,
    MODEL_TERRAIN_RIVER,
    MODEL_TERRAIN_ROAD,
    MODEL_TERRAIN_ROAD_CORNER,
    MODEL_TERRAIN_SEA,
    MODEL_TERRAIN_COUNT
};

struct ModelTransform {
    glm::vec3 position;
};

bool model_init();
bool model_load(Model* model, std::vector<std::string> paths);
bool model_texture_load(GLuint* texture, std::string path);

void model_unit_queue_render(ModelUnit model_unit, ModelUnitColor model_color, ModelTransform transform);
void model_unit_render_from_queues();
void model_terrain_queue_render(ModelTerrain model_terain, ModelTransform transform);
void model_terrain_render_from_queues();