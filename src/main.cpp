#ifdef _WIN32
    #define SDL_MAIN_HANDLED
#endif

#include "shader.hpp"
#include "font.hpp"

#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    }; 

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Set OpenGL flags
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

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
        glUseProgram(shader);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        font_render(font_hack10, "FPS: " + std::to_string(fps), 0, 0, (SDL_Color) { .r = 255, .g = 255, .b = 0, .a = 255 });

        SDL_GL_SwapWindow(window);
        frames++;
    }

    TTF_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}