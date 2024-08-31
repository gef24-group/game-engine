#pragma once

#include "App.hpp"
#include "SDL_render.h"
#include "Types.hpp"
#include <string>

extern App *app;

SDL_Texture *LoadTexture(std::string path);
void Log(LogLevel log_level, const char *fmt, ...);