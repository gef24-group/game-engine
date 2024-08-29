#pragma once

#include "GameObject.hpp"
#include "SDL_render.h"
#include "SDL_video.h"
#include <vector>

class GameEngine {
  private:
    // TODO: Make a struct named "App" in which win and ren are member variables.
    SDL_Window *win;
    SDL_Renderer *ren;
    std::vector<GameObject> gameObjects;

  public:
    GameEngine();
    void Start();
    bool Init(const char *game_title);
    bool InitializeDisplay(const char *game_title);
    void ShowWelcomeScreen(int red, int green, int blue);
    void AddObject(GameObject game_object);
    void Update();
    void PrepareScene();
    void PresentScene();
    void Shutdown();
};