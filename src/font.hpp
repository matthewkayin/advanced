#pragma once

#include <glad/glad.h>
#include <SDL2/SDL.h>

#include <string>

struct Font {
    GLuint atlas;
    unsigned int glyph_width;
    unsigned int glyph_height;
};

extern Font font_hack10;

bool font_init();
void font_render(const Font& font, std::string text, int x, int y, SDL_Color color);