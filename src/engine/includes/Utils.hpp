#pragma once

#include "App.hpp"
#include "GameObject.hpp"
#include "SDL_render.h"
#include "Types.hpp"
#include <string>

extern App *app;

SDL_Texture *LoadTexture(std::string path);
void Log(LogLevel log_level, const char *fmt, ...);
Size GetWindowSize();
GameObject *GetObjectByName(std::string name, std::vector<GameObject *> game_objects);