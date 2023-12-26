#include "font.hpp"

#include "shader.hpp"
#include "global.hpp"

#include <SDL2/SDL_ttf.h>

#include <cstdio>

const int FIRST_CHAR = 32;

Font font_hack10;
TTF_Font* test;

unsigned int glyph_vao;

bool font_load(Font* font, const char* path, unsigned int size);

int next_largest_power_of_two(int number) {
    int power_of_two = 1;
    while (power_of_two < number) {
        power_of_two *= 2;
    }

    return power_of_two;
}

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

    int atlas_width = next_largest_power_of_two(max_width * 96);
    int atlas_height = next_largest_power_of_two(max_height);
    SDL_Surface* atlas_surface = SDL_CreateRGBSurface(0, atlas_width, atlas_height, 32, 0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
    for (int i = 0; i < 96; i++) {
        SDL_Rect dest_rect = (SDL_Rect) { .x = max_width * i, .y = 0, .w = glyphs[i]->w, .h = glyphs[i]->h };
        SDL_BlitSurface(glyphs[i], NULL, atlas_surface, &dest_rect);
    }

    // Generate OpenGL texture
    glGenTextures(1, &font->atlas);
    glBindTexture(GL_TEXTURE_2D, font->atlas);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas_width, atlas_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, atlas_surface->pixels);

    // Finish setting up font struct
    font->glyph_width = (unsigned int)max_width;
    font->glyph_height = (unsigned int)max_height;

    // Cleanup
    glBindTexture(GL_TEXTURE_2D, 0);
    for (int i = 0; i < 96; i++) {
        SDL_FreeSurface(glyphs[i]);
    }
    SDL_FreeSurface(atlas_surface);
    TTF_CloseFont(ttf_font);

    return true;
}

  unsigned int power_two_floor(unsigned int val) {
    unsigned int power = 2, nextVal = power*2;

    while((nextVal *= 2) <= val)
      power*=2;

    return power*2;
  }

void font_render(const Font& font, std::string text, int x, int y, SDL_Color color) {
    glUseProgram(text_shader);
    float atlas_size[2] = { (float)next_largest_power_of_two(font.glyph_width * 96), (float)next_largest_power_of_two(font.glyph_height) };
    glUniform2fv(glGetUniformLocation(text_shader, "atlas_size"), 1, &atlas_size[0]);
    float render_size[2] = { (float)font.glyph_width, (float)font.glyph_height };
    glUniform2fv(glGetUniformLocation(text_shader, "render_size"), 1, &render_size[0]);
    float font_color[3] = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f };
    glUniform3fv(glGetUniformLocation(text_shader, "font_color"), 1, &font_color[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.atlas);
    glBindVertexArray(glyph_vao);

    float render_coords[2] = { (float)x, (float)y };
    float texture_offset[2] = { (float)0.0f, 0.0f };
    for (char c : text) {
        int glyph_index = (int)c - FIRST_CHAR;
        texture_offset[0] = (float)(font.glyph_width * glyph_index);
        glUniform2fv(glGetUniformLocation(text_shader, "render_coords"), 1, &render_coords[0]);
        glUniform2fv(glGetUniformLocation(text_shader, "texture_offset"), 1, &texture_offset[0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        render_coords[0] += font.glyph_width;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}