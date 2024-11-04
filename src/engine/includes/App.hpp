#pragma once

#include "SDL_render.h"
#include "Types.hpp"

typedef struct {
    SDL_Window *sdl_window;
    SDL_Renderer *renderer;
    std::atomic<bool> quit;
    std::atomic<bool> sigint;
    Window window;
} App;