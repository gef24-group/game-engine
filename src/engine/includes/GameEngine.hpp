#pragma once

#include "GameObject.hpp"
#include "SDL_render.h"
#include "SDL_video.h"
#include <functional>
#include <vector>

class GameEngine {
  private:
    // TODO: Make a struct named "App" in which win and ren are member variables.
    SDL_Window *win;
    SDL_Renderer *ren;
    std::vector<GameObject> game_objects;
    std::function<void(std::vector<GameObject> game_objects)> callback;

  public:
    GameEngine();
    void Start();
    bool Init(const char *game_title);
    bool InitializeDisplay(const char *game_title);
    void ShowWelcomeScreen(int red, int green, int blue);
    std::vector<GameObject> GetObjects();
    void AddObject(GameObject game_object);
    void SetCallback(std::function<void(std::vector<GameObject>)> callback);
    void Update(std::vector<GameObject>);
    void PrepareScene();
    void PresentScene();
    void Shutdown();
};