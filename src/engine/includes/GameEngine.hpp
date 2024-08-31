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
    std::function<void(std::vector<GameObject *> game_objects)> callback;
    KeyMap key_map;

  public:
    GameEngine();
    void Start();
    bool Init(const char *game_title);
    bool InitializeDisplay(const char *game_title);
    void SetBackgroundColor(Color color);
    void ShowWelcomeScreen();
    std::vector<GameObject *> GetObjects();
    void AddObjects(std::vector<GameObject *> game_objects);
    void SetCallback(std::function<void(std::vector<GameObject *>)> callback);
    void Update();
    void ReadHIDs();
    void ApplyObjectPhysics(float time);
    void ApplyObjectUpdates();
    void TestCollision();
    void StickCollidersToBoundary();
    void RenderScene();
    void RenderBackground();
    void RenderObject(GameObject *game_object);
    void Shutdown();
};