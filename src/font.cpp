#include "font.hpp"

#include "shader.hpp"
#include "global.hpp"

#include <SDL2/SDL_ttf.h>

#include <cstdio>

const unsigned int FIRST_CHAR = 32;

Font font_hack10;
TTF_Font* test;

unsigned int glyph_vao;

bool font_load(Font* font, const char* path, unsigned int size);

bool font_init() {
    if (TTF_Init() == -1) {
        printf("Unable to initialize SDL_ttf: %s\n", TTF_GetError());
        return false;
    }

    // buffer glyph vertex data
    unsigned int glyph_vbo;

    float glyph_vertices[12] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    glGenVertexArrays(1, &glyph_vao);
    glGenBuffers(1, &glyph_vbo);
    glBindVertexArray(glyph_vao);
    glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glyph_vertices), glyph_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (!font_load(&font_hack10, "./hack.ttf", 10)) {
        return false;
    }

    glUseProgram(text_shader);
    float screen_size[2] = { SCREEN_WIDTH, SCREEN_HEIGHT };
    glUniform2fv(glGetUniformLocation(text_shader, "screen_size"), 1, &screen_size[0]);
    glUniform1ui(glGetUniformLocation(text_shader, "u_texture"), 0);

    return true;
}

bool font_load(Font* font, const char* path, unsigned int size) {
    static const SDL_Color COLOR_WHITE = (SDL_Color) { .r = 255, .g = 255, .b = 255, .a = 255 };

    // Load the font
    TTF_Font* ttf_font = TTF_OpenFont(path, size);
    if (ttf_font == NULL) {
        printf("Unable to open font at path %s. SDL Error: %s\n", path, TTF_GetError());
        return false;
    }
    test = TTF_OpenFont(path, size);

    // Render each glyph to a surface
    SDL_Surface* glyphs[96];
    GLint internal_format;
    int max_width;
    int max_height;
    for (int i = 0; i < 96; i++) {
        char text[2] = { (char)(i + FIRST_CHAR), '\0' };
        glyphs[i] = TTF_RenderText_Blended(ttf_font, text, COLOR_WHITE);
        if (glyphs[i] == NULL) {
            return false;
        }

        if (i == 0 || max_width < glyphs[i]->w) {
            max_width = glyphs[i]->w;
        }
        if (i == 0 || max_height < glyphs[i]->h) {
            max_height = glyphs[i]->h;
        }
    }

    // Determine internal format
    if (glyphs[0]->format->BytesPerPixel == 4) {
        if (glyphs[0]->format->Rmask == 0x000000ff) {
            internal_format = GL_RGBA;
            printf("internal format is rgba\n");
        } else {
            internal_format = GL_BGRA;
            printf("internal format is bgra\n");
        }
    } else {
        if (glyphs[0]->format->Rmask == 0x000000ff) {
            internal_format = GL_RGB;
            printf("internal format is rgb\n");
        } else {
            internal_format = GL_BGR;
            printf("internal format is bgr\n");
        }
    }


    // Generate OpenGL texture
    GLubyte empty_data[max_width * 96 * max_height * 4];
    glGenTextures(1, &font->atlas);
    glBindTexture(GL_TEXTURE_2D, font->atlas);
    glTexImage2D(GL_TEXTURE_2D, 0, glyphs[0]->format->BytesPerPixel, max_width * 96, max_height, 0, internal_format, GL_UNSIGNED_BYTE, empty_data);

    // Render each glyph onto the texture atlas
    for (int i = 0; i < 96; i++) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, max_width * i, 0, max_width, max_height, internal_format, GL_UNSIGNED_BYTE, glyphs[i]->pixels);
    }

    // Finish setting up font struct
    font->glyph_width = (unsigned int)max_width;
    font->glyph_height = (unsigned int)max_height;

    // Cleanup
    glBindTexture(GL_TEXTURE_2D, 0);
    for (int i = 0; i < 96; i++) {
        SDL_FreeSurface(glyphs[i]);
    }
    TTF_CloseFont(ttf_font);

    return true;
}

  unsigned int power_two_floor(unsigned int val) {
    unsigned int power = 2, nextVal = power*2;

    while((nextVal *= 2) <= val)
      power*=2;

    return power*2;
  }

void font_render(const Font& font, std::string text, int x, int y) {
    /*glUseProgram(text_shader);
    float render_coords[2] = { x, y };
    glUniform2fv(glGetUniformLocation(text_shader, "render_coords"), 1, &render_coords[0]);
    float render_size[2] = { (float)font.glyph_width * 96.0f, font.glyph_height };
    glUniform2fv(glGetUniformLocation(text_shader, "render_size"), 1, &render_size[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.atlas);
    glBindVertexArray(glyph_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);*/

    SDL_Surface* test_surface = TTF_RenderText_Blended(test, "hello", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 });
    unsigned int test_w = power_two_floor(test_surface->w) * 2;
    unsigned int test_h = power_two_floor(test_surface->h) * 2;
    SDL_Surface* other = SDL_CreateRGBSurface(0, test_w, test_h, 32, 0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
    SDL_BlitSurface(test_surface, NULL, other, NULL);

    GLuint texture;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, other->w, other->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, other->pixels);

    glUseProgram(text_shader);
    float render_coords[2] = { x, y };
    glUniform2fv(glGetUniformLocation(text_shader, "render_coords"), 1, &render_coords[0]);
    float render_size[2] = { (float)test_surface->w, (float)test_surface->h };
    glUniform2fv(glGetUniformLocation(text_shader, "render_size"), 1, &render_size[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(glyph_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}