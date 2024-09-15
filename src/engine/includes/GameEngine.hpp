#pragma once

#include "App.hpp"
#include "GameObject.hpp"
#include "Timeline.hpp"
#include "Types.hpp"
#include <functional>
#include <vector>

extern App *app;

class GameEngine {
  private:
    Timeline engine_timeline;
    NetworkInfo network_info;
    Color background_color;
    std::vector<GameObject *> game_objects;
    std::function<void(std::vector<GameObject *> *game_objects)> callback;
    Window window;

  public:
    GameEngine();
    void Start();
    bool Init(const char *game_title);
    bool InitializeDisplay(const char *game_title);
    void SetNetworkInfo(NetworkInfo network_info);
    void SetBackgroundColor(Color color);
    void ShowWelcomeScreen();
    std::vector<GameObject *> GetObjects();
    void AddObjects(std::vector<GameObject *> game_objects);
    void SetCallback(std::function<void(std::vector<GameObject *> *)> callback);
    void Update();
    void GetTimeDelta();
    void ReadHIDs();
    void ApplyObjectPhysics();
    void ApplyObjectUpdates();
    void TestCollision();
    bool HandleEvents();
    void HandleCollisions();
    void HandleScaling();
    void HandleTimelineInputs();
    void RenderScene();
    void RenderBackground();
    void Shutdown();
};