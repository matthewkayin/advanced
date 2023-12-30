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
glm::vec3 light_pos = glm::vec3(-1.5f, 2.0f, 1.0f);

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
    glUniform1f(glGetUniformLocation(shader, "point_light.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shader, "point_light.quadratic"), 0.032f);

    glUseProgram(light_shader);
    glUniformMatrix4fv(glGetUniformLocation(light_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    GLuint cube_vbo;
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &cube_vbo);
    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
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
    glUseProgram(shader);
    glm::mat4 view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(glGetUniformLocation(shader, "view_pos"), 1, glm::value_ptr(camera_position));
    glActiveTexture(GL_TEXTURE0);

    glBlendFunc(GL_ONE, GL_ZERO);
    model_unit_queue_render(MODEL_UNIT_TANK, MODEL_UNIT_COLOR_BLUE, (ModelTransform) {
        .position = glm::vec3(0.0f, -0.5, 0.0f),
    }); 
    model_unit_render_from_queues();
    model_terrain_queue_render(MODEL_TERRAIN_FOREST, (ModelTransform) {
        .position = glm::vec3(0.0f, 0.0f, 0.0f)
    });
    model_terrain_render_from_queues();

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