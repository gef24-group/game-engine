#pragma once

#include "SDL_render.h"

// Referred from the sample code provided on Moodle:
// https://github.ncsu.edu/acard/SDL2-Window-Sample
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} App;