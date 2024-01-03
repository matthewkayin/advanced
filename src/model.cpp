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

bool model_init() {
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

        if (words.size() > 5) {
            printf("in object %s:", current_object.c_str());
            for (std::string word : words) {
                printf("%s ", word.c_str());
            }
            printf("\n");
        }

        // parse line
        if (words[0] == "v") {
            positions.push_back(glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3])));
        } else if (words[0] == "vt") {
            texture_coordinates.push_back(glm::vec2(std::stof(words[1]), std::stof(words[2])));
        } else if (words[0] == "vn") {
            normals.push_back(glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3])));
        } else if (words[0] == "f") {
            for (unsigned int face_index = 0; face_index < words.size() - 3; face_index++) {
                Face face;
                for (unsigned int i = face_index; i < face_index + 3; i++) {
                    std::size_t slash_index = words[i + 1].find("/");
                    std::size_t second_slash_index = words[i + 1].find("/", slash_index + 1);
                    // subtract all indices by one because OBJ file indices start at 1 instead of 0
                    face.position_indices[i] = std::stoi(words[i + 1].substr(0, slash_index)) - 1;
                    face.texture_coordinate_indices[i] = std::stoi(words[i + 1].substr(slash_index + 1, second_slash_index)) - 1;
                    face.normal_indices[i] = std::stoi(words[i + 1].substr(second_slash_index + 1)) - 1;
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
        }
    }
    filein.close();

    for (std::map<std::string, std::vector<Face>>::iterator it = faces.begin(); it != faces.end(); ++it) {
        printf("beginning mesh %s\n", it->first.c_str());
        std::vector<VertexData> vertex_data;
        for (Face face : it->second) {
            for (unsigned int i = 0; i < 3; i++) {
                if (face.position_indices[i] > positions.size()) {
                    printf("positions out of bounds\n");
                }
                if (face.normal_indices[i] > positions.size()) {
                    printf("indices out of bounds\n");
                }
                if (face.texture_coordinate_indices[i] > positions.size()) {
                    printf("tex coords out of bounds\n");
                }
                vertex_data.push_back((VertexData) {
                    .position = positions[face.position_indices[i]],
                    .normal = normals[face.normal_indices[i]],
                    .texture_coordinates = texture_coordinates[face.texture_coordinate_indices[i]]
                });
            }
        }

        printf("sending vertex data\n");
        Mesh new_mesh;
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

void model_render(Model& model, glm::vec3 position) {
    glUseProgram(shader);
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));

    for (std::map<std::string, Mesh>::iterator it = model.mesh.begin(); it != model.mesh.end(); ++it) {
        glBindVertexArray(it->second.vao);
        glDrawArrays(GL_TRIANGLES, 0, it->second.vertex_data_size);
    }
    glBindVertexArray(0);
}