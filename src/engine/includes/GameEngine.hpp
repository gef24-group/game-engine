#pragma once

#include "App.hpp"
#include "GameObject.hpp"
#include <functional>
#include <vector>

extern App *app;

class GameEngine {
  private:
    Color background_color;
    std::vector<GameObject *> game_objects;
    std::function<void(std::vector<GameObject *> *game_objects)> callback;
    Window window;

  public:
    GameEngine();
    void Start();
    bool Init(const char *game_title);
    bool InitializeDisplay(const char *game_title);
    void SetBackgroundColor(Color color);
    void ShowWelcomeScreen();
    std::vector<GameObject *> GetObjects();
    void AddObjects(std::vector<GameObject *> game_objects);
    void SetCallback(std::function<void(std::vector<GameObject *> *)> callback);
    void Update();
    void ReadHIDs();
    void ApplyObjectPhysics(float time);
    void ApplyObjectUpdates();
    void TestCollision();
    bool HandleEvents();
    void HandleCollisions();
    void HandleScaling();
    void RenderScene();
    void RenderBackground();
    void Shutdown();
};