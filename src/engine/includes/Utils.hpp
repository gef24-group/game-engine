#pragma once

#include "App.hpp"
#include "SDL_render.h"
#include <string>

extern App *app;
SDL_Texture *LoadTexture(std::string path);