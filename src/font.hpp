#pragma once

#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include <string>

struct Font {
    GLuint atlas;
    unsigned int glyph_width;
    unsigned int glyph_height;
};

extern Font font_hack10;

const glm::vec3 FONT_COLOR_WHITE = glm::vec3(1.0f, 1.0f, 1.0f);

bool font_init();
void font_render(const Font& font, std::string text, glm::vec2 render_pos, glm::vec3 color);