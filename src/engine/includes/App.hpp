#pragma once

#include "SDL_render.h"
#include "Types.hpp"

// Referred from the sample code provided on Moodle:
// https://github.ncsu.edu/acard/SDL2-Window-Sample
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool quit;
    KeyMap *key_map;
} App;