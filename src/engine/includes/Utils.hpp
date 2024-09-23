#pragma once

#include "App.hpp"
#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "SDL_render.h"
#include "Types.hpp"
#include <string>

extern App *app;

SDL_Texture *LoadTexture(std::string path);
void Log(LogLevel log_level, const char *fmt, ...);
Size GetWindowSize();
GameObject *GetObjectByName(std::string name, std::vector<GameObject *> game_objects);
GameObject *GetControllable(std::vector<GameObject *> game_objects);
std::vector<GameObject *> GetObjectsByRole(NetworkInfo network_info,
                                           std::vector<GameObject *> game_objects);
void SetPlayerTexture(GameObject *controllable, int player_id);
GameObject *GetClientPlayer(int player_id, std::vector<GameObject *> game_objects);
bool SetEngineCLIOptions(GameEngine *game_engine, int argc, char *args[]);
void HandleSIGINT(int signum);
int GetPlayerIdFromName(std::string player_name);
std::vector<std::string> Split(std::string str, char delimiter);