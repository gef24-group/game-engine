#pragma once

#include "SDL_render.h"
#include "Types.hpp"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool quit;
    std::atomic<bool> sigint;
    KeyMap *key_map;
} App;