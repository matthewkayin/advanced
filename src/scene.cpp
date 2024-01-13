#include "scene.hpp"

#include "shader.hpp"
#include "model.hpp"
#include "global.hpp"

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
float camera_yaw = -90.0f;
float camera_pitch = 0.0f;

const Uint8* keys;
GLuint cube_vao;
GLuint floor_vao;
GLuint floor_texture;
glm::vec3 light_pos = glm::vec3(-5.0f, 10.0f, 1.0f);
Model car_model;

void scene_generate_cube(GLuint* vao, glm::vec3 size);

void scene_init() {
    keys = SDL_GetKeyboardState(NULL);

    glUseProgram(shader);
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / float(SCREEN_HEIGHT), 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shader, "model_texture"), 0);
    
    // light uniforms
    glUniform3fv(glGetUniformLocation(shader, "point_light.position"), 1, glm::value_ptr(light_pos));
    glUniform1f(glGetUniformLocation(shader, "point_light.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shader, "point_light.linear"), 0.022f);
    glUniform1f(glGetUniformLocation(shader, "point_light.quadratic"), 0.0019f);

    glUseProgram(light_shader);
    glUniformMatrix4fv(glGetUniformLocation(light_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    model_load(&car_model, "./res/car/car.obj");
    scene_generate_cube(&cube_vao, glm::vec3(0.5f));
    scene_generate_cube(&floor_vao, glm::vec3(100.0f, 0.01f, 100.0f));
    model_texture_load(&floor_texture, "./res/floor.png");
}

void scene_handle_input(SDL_Event e) {
    if (e.type == SDL_MOUSEMOTION) {
        const float sensitivity = 0.1f;
        camera_yaw += e.motion.xrel * sensitivity;
        camera_pitch -= e.motion.yrel * sensitivity;
        if (camera_pitch > 89.0f) {
            camera_pitch = 89.0f;
        } else if (camera_pitch < -89.0f) {
            camera_pitch = -89.0f;
        }

        camera_front = glm::normalize(glm::vec3(
            cos(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch)),
            sin(glm::radians(camera_pitch)),
            sin(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch))
        ));
    }
}

void scene_update(float delta) {
    glm::vec3 camera_direction = glm::vec3(0.0f);
    if (keys[SDL_SCANCODE_W]) {
        camera_direction.z = 1.0f;
    }
    if (keys[SDL_SCANCODE_S]) {
        camera_direction.z = -1.0f;
    }
    if (keys[SDL_SCANCODE_A]) {
        camera_direction.x = -1.0f;
    }
    if (keys[SDL_SCANCODE_D]) {
        camera_direction.x = 1.0f;
    }
    if (keys[SDL_SCANCODE_Q]) {
        camera_direction.y = -1.0f;
    }
    if (keys[SDL_SCANCODE_E]) {
        camera_direction.y = 1.0f;
    }

    const float CAMERA_SPEED = 1.0f;
    glm::vec3 camera_velocity = glm::vec3(0.0f);
    camera_velocity += camera_front * camera_direction.z;
    camera_velocity += camera_up * camera_direction.y;
    camera_velocity += glm::cross(camera_front, camera_up) * camera_direction.x;
    
    camera_position += camera_velocity * CAMERA_SPEED * delta;
}

void scene_render() {
    glActiveTexture(GL_TEXTURE0);
    glBlendFunc(GL_ONE, GL_ZERO);

    glm::mat4 view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(glGetUniformLocation(shader, "view_pos"), 1, glm::value_ptr(camera_position));

    model_render(car_model, glm::vec3(0.0f));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floor_texture);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, model_null_texture);
    glm::mat4 floor_model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(floor_model));
    glUniform3fv(glGetUniformLocation(shader, "material.ka"), 1, glm::value_ptr(glm::vec3(0.5)));
    glUniform3fv(glGetUniformLocation(shader, "material.kd"), 1, glm::value_ptr(glm::vec3(0.8)));
    glUniform3fv(glGetUniformLocation(shader, "material.ks"), 1, glm::value_ptr(glm::vec3(1.0)));
    glUniform1i(glGetUniformLocation(shader, "material.map_ka"), 0);
    glUniform1i(glGetUniformLocation(shader, "material.map_kd"), 1);
    glBindVertexArray(floor_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glUseProgram(light_shader);
    glUniformMatrix4fv(glGetUniformLocation(light_shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(glGetUniformLocation(light_shader, "view_pos"), 1, glm::value_ptr(camera_position));
    glm::mat4 light_model = glm::mat4(1.0f);
    light_model = glm::translate(light_model, light_pos);
    light_model = glm::scale(light_model, glm::vec3(0.25f));
    glUniformMatrix4fv(glGetUniformLocation(light_shader, "model"), 1, GL_FALSE, glm::value_ptr(light_model));
    glBindVertexArray(cube_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void scene_generate_cube(GLuint* vao, glm::vec3 size) {
    float vertices[] = {
        // positions          // normals           // texture coords
        -size.x, -size.y, -size.z,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         size.x, -size.y, -size.z,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         size.x,  size.y, -size.z,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         size.x,  size.y, -size.z,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -size.x,  size.y, -size.z,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -size.x, -size.y, -size.z,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -size.x, -size.y,  size.z,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         size.x, -size.y,  size.z,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         size.x,  size.y,  size.z,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         size.x,  size.y,  size.z,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -size.x,  size.y,  size.z,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -size.x, -size.y,  size.z,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -size.x,  size.y,  size.z, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -size.x,  size.y, -size.z, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -size.x, -size.y, -size.z, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -size.x, -size.y, -size.z, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -size.x, -size.y,  size.z, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -size.x,  size.y,  size.z, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         size.x,  size.y,  size.z,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         size.x,  size.y, -size.z,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         size.x, -size.y, -size.z,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         size.x, -size.y, -size.z,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         size.x, -size.y,  size.z,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         size.x,  size.y,  size.z,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -size.x, -size.y, -size.z,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         size.x, -size.y, -size.z,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         size.x, -size.y,  size.z,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         size.x, -size.y,  size.z,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -size.x, -size.y,  size.z,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -size.x, -size.y, -size.z,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -size.x,  size.y, -size.z,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         size.x,  size.y, -size.z,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         size.x,  size.y,  size.z,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         size.x,  size.y,  size.z,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -size.x,  size.y,  size.z,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -size.x,  size.y, -size.z,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    GLuint vbo;
    glGenVertexArrays(1, vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
}