#pragma once

#include "App.hpp"
#include "GameObject.hpp"
#include "Timeline.hpp"
#include "Types.hpp"
#include <functional>
#include <vector>
#include <zmq.hpp>

extern App *app;

class GameEngine {
  private:
    std::string game_title;
    Timeline engine_timeline;
    NetworkInfo network_info;
    std::atomic<int> clients_connected = -1;
    Color background_color;
    std::vector<GameObject *> game_objects;
    std::function<void(std::vector<GameObject *> *game_objects)> callback;
    Window window;
    bool InitCSClientConnection();
    void ListenServerBroadcasts(zmq::context_t &context);

  public:
    GameEngine();
    bool Init();
    bool InitSingleClient();
    bool InitCSServer();
    bool InitCSClient();
    bool InitP2PServer();
    bool InitP2PPeer();
    void Start();
    void StartSingleClient();
    void StartCSServer();
    void StartCSClient();
    void StartP2PServer();
    void StartP2PPeer();
    void SetupDefaultInputs();
    bool InitializeDisplay();
    void SetGameTitle(std::string game_title);
    void SetNetworkInfo(NetworkInfo network_info);
    NetworkInfo GetNetworkInfo();
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
    void RenderScene();
    void RenderBackground();
    void Shutdown();
};