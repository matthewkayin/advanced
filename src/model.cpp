#include "model.hpp"

#include "shader.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <fstream>
#include <cstdio>
#include <queue>
#include <map>

GLuint model_unit_texture[MODEL_UNIT_COLOR_COUNT];
Model model_unit[MODEL_UNIT_COUNT];
std::queue<ModelTransform> model_unit_render_queue[MODEL_UNIT_COUNT * MODEL_UNIT_COLOR_COUNT];

const std::string TERRAIN_PATH_PREFIX = "./res/model/Terrain/";
const std::map<ModelTerrain, std::string> TERRAIN_PATH_SUFFIX = {
    { MODEL_TERRAIN_AIRPORT, "Airport.obj" },
    { MODEL_TERRAIN_BASE, "Base.obj" },
    { MODEL_TERRAIN_BEACH, "Beach.obj" },
    { MODEL_TERRAIN_BEACH_INNER_CORNER, "Beach_InnerCorner.obj" },
    { MODEL_TERRAIN_BEACH_OUTER_CORNER, "Beach_OuterCorner.obj" },
    { MODEL_TERRAIN_BRIDGE, "Bridge.obj" },
    { MODEL_TERRAIN_CITY, "City.obj" },
    { MODEL_TERRAIN_FOREST, "Forest.obj" },
    { MODEL_TERRAIN_MOUNTAIN_HIGH, "Mountain_High.obj" },
    { MODEL_TERRAIN_MOUNTAIN_LOW, "Mountain_low.obj" },
    { MODEL_TERRAIN_QG, "QG.obj" },
    { MODEL_TERRAIN_RIVER, "River.obj" },
    { MODEL_TERRAIN_ROAD, "Road.obj" },
    { MODEL_TERRAIN_ROAD_CORNER, "Road_Corner.obj" },
    { MODEL_TERRAIN_SEA, "Sea.obj" }
};

GLuint model_terrain_texture;
Model model_terrain[MODEL_TERRAIN_COUNT];
std::queue<ModelTransform> model_terrain_render_queue[MODEL_TERRAIN_COUNT];

bool model_init() {
    if (!model_texture_load(&model_unit_texture[MODEL_UNIT_COLOR_BLUE], "./res/texture/Units_Blue.png")) {
        return false;
    }
    if (!model_texture_load(&model_unit_texture[MODEL_UNIT_COLOR_RED], "./res/texture/Units_Red.png")) {
        return false;
    }
    if (!model_texture_load(&model_terrain_texture, "./res/texture/Terrain.png")) {
        return false;
    }
    if (!model_load(&model_unit[MODEL_UNIT_TANK], std::vector<std::string> { "./res/model/Tank/Tank-0.obj", "./res/model/Tank/Tank-1.obj", "./res/model/Tank/Tank-2.obj" })) {
        return false;
    }
    for (unsigned int i = 0; i < MODEL_TERRAIN_COUNT; i++) {
        if (!model_load(&model_terrain[i], std::vector<std::string> { TERRAIN_PATH_PREFIX + TERRAIN_PATH_SUFFIX.at((ModelTerrain)i) } )) {
            return false;
        }
    }
    return true;
}

bool model_load(Model* model, std::vector<std::string> paths) {
    // define structs
    struct Face {
        unsigned int position_indices[3];
        unsigned int texture_coordinate_indices[3];
        unsigned int normal_indices[3];
    };
    struct VertexData {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texture_coordinates;
    };

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texture_coordinates;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;

    unsigned int position_base_index = 0;
    unsigned int texture_coordinate_base_index = 0;
    unsigned int normal_base_index = 0;

    for (std::string path : paths) {
        // open file
        std::ifstream filein;
        filein.open(path.c_str());
        
        if (!filein.is_open()) {
            printf("Unable to open model %s\n", path.c_str());
            return false;
        }

        position_base_index = positions.size();
        texture_coordinate_base_index = texture_coordinates.size();
        normal_base_index = normals.size();

        std::string line;
        while (std::getline(filein, line)) {
            // ignore comments
            if (line == "" || line[0] == '#') {
                continue;
            }

            // split line into words
            std::vector<std::string> words;
            while (line != "") {
                std::size_t space_index = line.find(" ");
                if (space_index == std::string::npos) {
                    words.push_back(line);
                    line = "";
                } else {
                    words.push_back(line.substr(0, space_index));
                    line = line.substr(space_index + 1);
                }
            }

            // parse line
            if (words[0] == "v") {
                positions.push_back(glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3])));
            } else if (words[0] == "vt") {
                texture_coordinates.push_back(glm::vec2(std::stof(words[1]), std::stof(words[2])));
            } else if (words[0] == "vn") {
                normals.push_back(glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3])));
            } else if (words[0] == "f") {
                Face face;
                for (unsigned int i = 0; i < 3; i++) {
                    std::size_t slash_index = words[i + 1].find("/");
                    std::size_t second_slash_index = words[i + 1].find("/", slash_index + 1);
                    // subtract all indices by one because OBJ file indices start at 1 instead of 0
                    face.position_indices[i] = position_base_index + (std::stoi(words[i + 1].substr(0, slash_index)) - 1);
                    face.texture_coordinate_indices[i] = texture_coordinate_base_index + (std::stoi(words[i + 1].substr(slash_index + 1, second_slash_index)) - 1);
                    face.normal_indices[i] = normal_base_index + (std::stoi(words[i + 1].substr(second_slash_index + 1)) - 1);
                }
                faces.push_back(face);
            }
        }

        filein.close();
    }

    std::vector<VertexData> vertex_data;
    for (Face face : faces) {
        for (unsigned int i = 0; i < 3; i++) {
            vertex_data.push_back((VertexData) {
                .position = positions[face.position_indices[i]],
                .normal = normals[face.normal_indices[i]],
                .texture_coordinates = texture_coordinates[face.texture_coordinate_indices[i]]
            });
        }
    }

    glGenVertexArrays(1, &model->vao);
    glGenBuffers(1, &model->vbo);
    glBindVertexArray(model->vao);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(VertexData), &vertex_data[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    model->vertex_data_size = vertex_data.size();

    return true;
}

bool model_texture_load(GLuint* texture, std::string path) {
    SDL_Surface* texture_surface = IMG_Load(path.c_str());
    if (texture_surface == NULL) {
        printf("Unable to load model texture at path %s\n", path.c_str());
        return false;
    }

    GLenum texture_format;
    if (texture_surface->format->BytesPerPixel == 4) {
        if (texture_surface->format->Rmask == 0x000000ff) {
            texture_format = GL_RGBA;
        } else {
            texture_format = GL_BGRA;
        }
    } else if (texture_surface->format->BytesPerPixel == 3) {
        if (texture_surface->format->Rmask == 0x000000ff) {
            texture_format = GL_RGB;
        } else {
            texture_format = GL_BGR;
        }
    } else {
        printf("Texture format of texture %s not recognized\n", path.c_str());
        return false;
    }

    glGenTextures(1, texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, texture_format, texture_surface->w, texture_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_surface->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(texture_surface);

    return true;
} 

void model_unit_queue_render(ModelUnit model_unit_index, ModelUnitColor model_unit_color, ModelTransform transform) {
    model_unit_render_queue[(model_unit_index * MODEL_UNIT_COLOR_COUNT) + model_unit_color].push(transform);
}

void model_unit_render_from_queues() {
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);

    for (unsigned int model_index = 0; model_index < MODEL_UNIT_COUNT; model_index++) {
        bool has_bound_vao = false;
        for (unsigned int color_index = 0; color_index < MODEL_UNIT_COLOR_COUNT; color_index++) {
            unsigned int queue_index = (model_index * MODEL_UNIT_COLOR_COUNT) + color_index;
            if (model_unit_render_queue[queue_index].empty()) {
                continue;
            }

            if (!has_bound_vao) {
                glBindVertexArray(model_unit[model_index].vao);
                has_bound_vao = true;
            }

            glBindTexture(GL_TEXTURE_2D, model_unit_texture[color_index]);

            while (!model_unit_render_queue[queue_index].empty()) {
                ModelTransform transform = model_unit_render_queue[queue_index].front();
                model_unit_render_queue[queue_index].pop();

                // set uniforms
                glm::mat4 model_matrix = glm::mat4(1.0f);
                model_matrix = glm::translate(model_matrix, transform.position);
                glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));

                // draw
                glDrawArrays(GL_TRIANGLES, 0, model_unit[model_index].vertex_data_size);
            }
        }
    }

    glBindVertexArray(0);
}

void model_terrain_queue_render(ModelTerrain model_terrain_index, ModelTransform transform) {
    model_terrain_render_queue[model_terrain_index].push(transform);
}

void model_terrain_render_from_queues() {
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, model_terrain_texture);

    for (unsigned int i = 0; i < MODEL_TERRAIN_COUNT; i++) {
        if (model_terrain_render_queue[i].empty()) {
            continue;
        }

        glBindVertexArray(model_terrain[i].vao);
        while (!model_terrain_render_queue[i].empty()) {
            ModelTransform transform = model_terrain_render_queue[i].front();
            model_terrain_render_queue[i].pop();

            // set uniforms
            glm::mat4 model_matrix = glm::mat4(1.0f);
            model_matrix = glm::translate(model_matrix, transform.position);
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));

            // draw
            glDrawArrays(GL_TRIANGLES, 0, model_unit[i].vertex_data_size);
        }
    }
}