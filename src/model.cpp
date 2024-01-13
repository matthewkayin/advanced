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

GLuint model_null_texture;

bool model_init() {
    if (!model_texture_load(&model_null_texture, "./res/null_texture.png")) {
        return false;
    }
    return true;
}

bool model_load(Model* model, std::string path) {
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
    std::map<std::string, std::vector<Face>> faces;

    // open file
    std::ifstream filein;
    filein.open(path.c_str());
    
    if (!filein.is_open()) {
        printf("Unable to open model %s\n", path.c_str());
        return false;
    }

    std::string current_object = "";
    std::string mtllib_path = "";

    std::string line;
    while (std::getline(filein, line)) {
        // ignore comments
        if (line == "" || line[0] == '#') {
            continue;
        }

        // split line into words
        std::vector<std::string> words;
        while (line != "") {
            if (line[0] == ' ') {
                line = line.substr(1);
                continue;
            }
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
            for (unsigned int face_index = 1; face_index < words.size() - 2; face_index++) {
                Face face;
                for (unsigned int i = 0; i < 3; i++) {
                    unsigned int index = i == 0 ? 1 : face_index + i;
                    std::size_t slash_index = words[index].find("/");
                    std::size_t second_slash_index = words[index].find("/", slash_index + 1);
                    // subtract all indices by one because OBJ file indices start at 1 instead of 0
                    face.position_indices[i] = std::stoi(words[index].substr(0, slash_index)) - 1;
                    face.texture_coordinate_indices[i] = std::stoi(words[index].substr(slash_index + 1, second_slash_index)) - 1;
                    face.normal_indices[i] = std::stoi(words[index].substr(second_slash_index + 1)) - 1;
                }
                faces[current_object].push_back(face);
            }
        } else if (words[0] == "o") {
            if (words.size() > 1) {
                current_object = words[1];
            } else {
                current_object = "";
            }
            std::map<std::string, std::vector<Face>>::iterator it = faces.find(current_object);
            if (it == faces.end()) {
                std::vector<Face> new_face_vector;
                faces[current_object] = new_face_vector;
            }
        } else if (words[0] == "mtllib") {
            mtllib_path = "";
            for (unsigned int i = 1; i < words.size(); i++) {
                mtllib_path += words[i];
            }
        } else if (words[0] == "usemtl") {
            std::string mtl_name = "";
            for (unsigned int i = 1; i < words.size(); i++) {
                mtl_name += words[i];
            }
            model->mesh[current_object].material = mtl_name;
        }
    }
    filein.close();

    for (std::map<std::string, std::vector<Face>>::iterator it = faces.begin(); it != faces.end(); ++it) {
        // get vertex data from faces
        std::vector<VertexData> vertex_data;
        for (Face face : it->second) {
            for (unsigned int i = 0; i < 3; i++) {
                vertex_data.push_back((VertexData) {
                    .position = positions[face.position_indices[i]],
                    .normal = normals[face.normal_indices[i]],
                    .texture_coordinates = texture_coordinates[face.texture_coordinate_indices[i]]
                });
            }
        }

        // center vertex positions around 0,0 and save the offset
        glm::vec3 vertex_min = vertex_data[0].position;
        glm::vec3 vertex_max = vertex_data[0].position;
        for (VertexData v : vertex_data) {
            for (int i = 0; i < 3; i++) {
                vertex_min[i] = std::min(v.position[i], vertex_min[i]);
                vertex_max[i] = std::max(v.position[i], vertex_max[i]);
            }
        }
        glm::vec3 mesh_center = vertex_min + ((vertex_max - vertex_min) / 2.0f);
        for (VertexData& v : vertex_data) {
            v.position -= mesh_center;
        }

        Mesh new_mesh;
        new_mesh.offset = mesh_center;
        new_mesh.vertex_data_size = vertex_data.size();
        glGenVertexArrays(1, &new_mesh.vao);
        glGenBuffers(1, &new_mesh.vbo);
        glBindVertexArray(new_mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, new_mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(VertexData), &vertex_data[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        model->mesh[it->first] = new_mesh;
    }

    // Read mtl file
    std::size_t path_last_forward_slash = path.rfind('/');
    std::string path_folder = path_last_forward_slash == std::string::npos ? "./" : path.substr(0, path_last_forward_slash + 1);
    std::string mtl_path = path_folder + mtllib_path;
    std::ifstream mtl_filein;
    mtl_filein.open(mtl_path.c_str());

    if (!mtl_filein.is_open()) {
        printf("Unable to open material lib %s\n", mtllib_path.c_str());
        return false;
    }

    std::string current_material = "";

    while (std::getline(mtl_filein, line)) {
        // ignore comments
        if (line == "" || line[0] == '#') {
            continue;
        }

        // split line into words
        std::vector<std::string> words;
        while (line != "") {
            if (line[0] == ' ') {
                line = line.substr(1);
                continue;
            }
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
        if (words[0] == "newmtl") {
            std::string mtl_name = "";
            for (unsigned int i = 1; i < words.size(); i++) {
                mtl_name += words[i];
            }

            model->material[current_material] = (Material) {
                .ka = glm::vec3(0.2f),
                .kd = glm::vec3(0.8f),
                .ks = glm::vec3(1.0f),
                .map_ka = 0,
                .map_kd = 0
            };
        } else if (words[0] == "Ka") {
            model->material[current_material].ka = glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
        } else if (words[0] == "Kd") {
            model->material[current_material].kd = glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
        } else if (words[0] == "Ks") {
            model->material[current_material].ks = glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
        } else if (words[0] == "map_Ka") {
            // TODO support non-png textures
            if (words[1].find(".png") == std::string::npos) {
                continue;
            }

            model_texture_load(&model->material[current_material].map_ka, path_folder + words[1]);
        } else if (words[0] == "map_Kd") {
            // TODO support non-png textures
            if (words[1].find(".png") == std::string::npos) {
                continue;
            }

            model_texture_load(&model->material[current_material].map_kd, path_folder + words[1]);
        }
    }
    mtl_filein.close();

    return true;
}

bool model_texture_load(GLuint* texture, std::string path) {
    SDL_Surface* texture_surface = IMG_Load(path.c_str());
    if (texture_surface == NULL) {
        printf("Unable to load model texture at path %s: %s\n", path.c_str(), IMG_GetError());
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_surface->w, texture_surface->h, 0, texture_format, GL_UNSIGNED_BYTE, texture_surface->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(texture_surface);

    return true;
} 

void model_render(Model& model, ModelTransform& transform) {
    glUseProgram(shader);

    glm::mat4 base_model_matrix = transform.base.to_model();
    for (std::map<std::string, Mesh>::iterator it = model.mesh.begin(); it != model.mesh.end(); ++it) {
        glm::mat4 model_matrix = base_model_matrix * glm::translate(glm::mat4(1.0f), it->second.offset);
        if (transform.mesh.count(it->first)) {
            model_matrix = model_matrix * transform.mesh[it->first].to_model();
        }

        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniform3fv(glGetUniformLocation(shader, "material.ka"), 1, glm::value_ptr(model.material[it->second.material].ka));
        glUniform3fv(glGetUniformLocation(shader, "material.kd"), 1, glm::value_ptr(model.material[it->second.material].kd));
        glUniform3fv(glGetUniformLocation(shader, "material.ks"), 1, glm::value_ptr(model.material[it->second.material].ks));
        glActiveTexture(GL_TEXTURE0);
        if (model.material[it->second.material].map_ka != 0) {
            glBindTexture(GL_TEXTURE_2D, model.material[it->second.material].map_ka);
        // if no ambient map, try using diffuse map
        } else if (model.material[it->second.material].map_kd != 0) {
            glBindTexture(GL_TEXTURE_2D, model.material[it->second.material].map_kd);
        } else {
            glBindTexture(GL_TEXTURE_2D, model_null_texture);
        }
        glActiveTexture(GL_TEXTURE0 + 1);
        if (model.material[it->second.material].map_kd != 0) {
            glBindTexture(GL_TEXTURE_2D, model.material[it->second.material].map_kd);
        } else {
            glBindTexture(GL_TEXTURE_2D, model_null_texture);
        }
        glUniform1i(glGetUniformLocation(shader, "material.map_ka"), 0);
        glUniform1i(glGetUniformLocation(shader, "material.map_kd"), 1);

        glBindVertexArray(it->second.vao);
        glDrawArrays(GL_TRIANGLES, 0, it->second.vertex_data_size);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}