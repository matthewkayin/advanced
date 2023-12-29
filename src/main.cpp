#ifdef _WIN32
    #define SDL_MAIN_HANDLED
#endif

#include "shader.hpp"
#include "font.hpp"
#include "model.hpp"
#include "global.hpp"

#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

// Engine
SDL_Window* window;
SDL_GLContext context;

unsigned int WINDOW_WIDTH = 640;
unsigned int WINDOW_HEIGHT = 480;

const unsigned long FRAME_TIME = 1000.0 / 60.0;
unsigned long last_time = SDL_GetTicks();
unsigned long last_second = SDL_GetTicks();
unsigned int frames = 0;
unsigned int fps = 0;
float elapsed = 0.0f;

int main() {
    // Init engine
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return -1;
    }

    int img_flags = IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        printf("Error initializing SDL_image: %s\n", IMG_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_LoadLibrary(NULL);

    window = SDL_CreateWindow("strategy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (window == NULL) {
        printf("Error creating window: %s\n", SDL_GetError());
        return -1;
    }

    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        printf("Error creating gl context: %s\n", SDL_GetError());
        return -1;
    }

    gladLoadGLLoader(SDL_GL_GetProcAddress);
    printf("Initialized OpenGL. Vendor %s\nRenderer %s\nVersion%s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));

    if (glGenVertexArrays == NULL) {
        printf("glGenVertexArrays is null, so there must have been a problem loading OpenGL.\n");
        return -1;
    }

    if (!shader_init()) {
        return -1;
    }
    if (!font_init()) {
        return -1;
    }
    if (!model_init()) {
        return -1;
    }

    // Set OpenGL flags
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glUseProgram(shader);
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / float(SCREEN_HEIGHT), 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, -3.0f);
    glUniform3fv(glGetUniformLocation(shader, "view_pos"), 1, glm::value_ptr(view_pos));
    glUniform1i(glGetUniformLocation(shader, "model_texture"), 0);
    
    // light uniforms
    glm::vec3 light_pos = glm::vec3(-1.0f, 1.0f, 1.0f);
    glUniform3fv(glGetUniformLocation(shader, "point_light.position"), 1, glm::value_ptr(light_pos));
    glUniform1f(glGetUniformLocation(shader, "point_light.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shader, "point_light.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shader, "point_light.quadratic"), 0.032f);

    // Game loop
    bool running = true;
    while (running) {
        // Timekeep
        unsigned long current_time = SDL_GetTicks();
        if (current_time - last_time < FRAME_TIME) {
            continue;
        }

        float delta = (float)(current_time - last_time) / 60.0f;
        last_time = current_time;

        if (current_time - last_second >= 1000) {
            fps = frames;
            frames = 0;
            last_second += 1000;
        }

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        // Update

        // Render
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBlendFunc(GL_ONE, GL_ZERO);
        model_unit_queue_render(MODEL_UNIT_TANK, MODEL_UNIT_COLOR_BLUE, (ModelTransform) {
            .position = glm::vec3(0.0f, -1.0f, 0.0f),
        });
        model_unit_render_from_queues();
        model_terrain_queue_render(MODEL_TERRAIN_BASE, (ModelTransform) {
            .position = glm::vec3(0.0f, 0.0f, 0.0f)
        });
        model_terrain_render_from_queues();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        font_render(font_hack10, "FPS: " + std::to_string(fps), glm::vec2(0.0f, 0.0f), FONT_COLOR_WHITE);

        SDL_GL_SwapWindow(window);
        frames++;
    }

    TTF_Quit();
    IMG_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}