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
    std::atomic<int> players_connected;
    Color background_color;
    bool show_player_border;
    int player_textures;
    int max_players;
    std::vector<GameObject *> game_objects;
    std::function<void(std::vector<GameObject *> *game_objects)> callback;

    zmq::context_t zmq_context;
    zmq::socket_t join_socket;
    zmq::socket_t server_broadcast_socket;
    zmq::socket_t client_update_socket;
    zmq::socket_t host_broadcast_socket;
    zmq::socket_t peer_broadcast_socket;

    bool InitSingleClient();
    bool InitCSServer();
    bool InitCSClient();
    bool InitP2PHost();
    bool InitP2PPeer();

    void StartSingleClient();
    void StartCSServer();
    void StartCSClient();
    void StartP2P();

    bool InitializeDisplay();
    bool InitCSClientConnection();
    void ShowWelcomeScreen();

    void CSServerClientThread(JoinReply join_reply);
    void CSServerBroadcastUpdates();
    void CSServerListenerThread();
    void CSClientAddExistingPlayers();
    void CSClientReceiveBroadcastThread();
    void CSClientSendUpdate();
    GameObject *CreateNewPlayer(int player_id);

    bool InitP2PPeerConnection();

    void P2PHostListenerThread();
    void P2PHostBroadcastPlayers();
    void P2PReceiveBroadcastThread(int player_id);
    void P2PReceiveBroadcastFromHostThread();
    void P2PBroadcastUpdates();

    void SetupDefaultInputs();
    void ReadInputsThread();
    bool HandleQuitEvent();
    void GetTimeDelta();
    void ApplyObjectPhysicsAndUpdates();
    void TestCollision();
    void HandleCollisions();
    void Update();
    void HandleScaling();
    void RenderScene();
    void RenderBackground();
    void Shutdown();

  public:
    GameEngine();
    bool Init();
    void Start();
    void SetGameTitle(std::string game_title);
    void SetNetworkInfo(NetworkInfo network_info);
    NetworkInfo GetNetworkInfo();
    void SetBackgroundColor(Color color);
    void SetShowPlayerBorder(bool show_player_border);
    void SetPlayerTextures(int player_textures);
    void SetMaxPlayers(int max_players);
    void AddObjects(std::vector<GameObject *> game_objects);
    void SetCallback(std::function<void(std::vector<GameObject *> *)> callback);
};