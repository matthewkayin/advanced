#pragma once

#include <SDL2/SDL.h>

void scene_init();
void scene_handle_input(SDL_Event e);
void scene_update(float delta);
void scene_render();