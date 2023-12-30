#ifdef _WIN32
    #define SDL_MAIN_HANDLED
#endif

#include "shader.hpp"
#include "font.hpp"
#include "model.hpp"
#include "global.hpp"
#include "scene.hpp"

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

unsigned int WINDOW_WIDTH = SCREEN_WIDTH * 2;
unsigned int WINDOW_HEIGHT = SCREEN_HEIGHT * 2;

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
    scene_init();

    // Set OpenGL flags
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    // Setup screen shader
    glUseProgram(screen_shader);
    glUniform1i(glGetUniformLocation(screen_shader, "screen_texture"), 0);

    // Setup quad vao
    GLuint quad_vao;
    GLuint quad_vbo;

    float quad_vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);

    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    // Setup framebuffer
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLuint texture_color_buffer;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_color_buffer);
    glBindTexture(GL_TEXTURE_2D, texture_color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color_buffer, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer not complete!\n");
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
            } else if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            } else {
                scene_handle_input(e);
            }
        }

        // Update
        scene_update(delta);

        // Render
        // Prepare rendering onto framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glBlendFunc(GL_ONE, GL_ZERO);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.05f, 0.05f, 0.05f, 0.05f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render scene
        scene_render();

        // Render framebuffer to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBlendFunc(GL_ONE, GL_ZERO);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(screen_shader);
        glBindVertexArray(quad_vao);
        glBindTexture(GL_TEXTURE_2D, texture_color_buffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Render fps
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