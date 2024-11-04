#pragma once

#include "App.hpp"
#include "Entity.hpp"
#include "SDL_render.h"
#include "Types.hpp"
#include <string>

extern App *app;

SDL_Texture *LoadTexture(std::string path);
void Log(LogLevel log_level, const char *fmt, ...);
Size GetWindowSize();
Overlap GetOverlap(SDL_Rect rect_1, SDL_Rect rect_2);
Entity *GetEntityByName(std::string name, std::vector<Entity *> entities);
Entity *GetControllable(std::vector<Entity *> entities);
int GetControllableCount(std::vector<Entity *> entities);
std::vector<Entity *> GetEntitiesByRole(NetworkInfo network_info, std::vector<Entity *> entities);
void SetPlayerTexture(Entity *controllable, int player_id, int player_textures);
Entity *GetClientPlayer(int player_id, std::vector<Entity *> entities);
bool SetEngineCLIOptions(int argc, char *args[]);
std::string GetConnectionAddress(std::string address, int port);
void HandleSIGINT(int signum);
Position GetScreenPosition(Position world_position, Position camera_position);
int GetPlayerIdFromName(std::string player_name);
std::vector<std::string> Split(std::string str, char delimiter);
bool IsZoneCategory(EntityCategory category);
std::vector<Entity *> GetEntitiesByCategory(std::vector<Entity *> entities,
                                            EntityCategory category);
int GetRandomInt(int n);