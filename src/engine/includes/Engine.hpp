#pragma once

#include "App.hpp"
#include "Entity.hpp"
#include "Timeline.hpp"
#include "Types.hpp"
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <zmq.hpp>

extern App *app;

class Engine {
  private:
    std::string title;
    std::shared_ptr<Timeline> engine_timeline;
    NetworkInfo network_info;
    std::atomic<int> players_connected;
    Color background_color;
    bool show_player_border;
    bool show_zone_borders;
    int player_textures;
    int max_players;
    std::shared_ptr<Entity> camera;

    std::mutex entities_mutex;
    std::vector<Entity *> entities;
    std::function<void(std::vector<Entity *> *)> callback;

    std::thread input_thread;
    std::thread listener_thread;
    std::thread receive_broadcast_thread;
    std::vector<std::thread> client_threads;
    std::vector<std::thread> peer_threads;

    std::atomic<bool> stop_input_thread;
    std::atomic<bool> stop_listener_thread;
    std::atomic<bool> stop_receive_broadcast_thread;
    std::atomic<bool> stop_client_thread;
    std::atomic<bool> stop_peer_thread;

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

    void CSServerClientThread(int player_id);
    void CSServerBroadcastUpdates();
    void CSServerListenerThread();
    void CSClientAddExistingPlayers();
    void CSClientReceiveBroadcastThread();
    void CSClientSendUpdate();

    Entity *CreateNewPlayer(int player_id, std::string player_address = "");

    bool InitP2PPeerConnection();

    void P2PHostListenerThread();
    void P2PHostBroadcastPlayers();
    void P2PReceiveBroadcastFromPeerThread(int player_id, std::string player_address);
    void P2PReceiveBroadcastFromHostThread();
    void P2PBroadcastUpdates();

    void SetupDefaultInputs();
    void ReadInputsThread();
    bool HandleQuitEvent();
    void GetTimeDelta();
    void ApplyEntityPhysicsAndUpdates();
    void TestCollision();
    void HandleCollisions();
    void HandleSideBoundaries();
    bool HandleDeathZones();
    void ResetSideBoundaries();
    void SetSideBoundaryVelocities(Velocity velocity);
    void Update();
    void HandleScaling();
    void RenderScene();
    void RenderBackground();
    void Shutdown();

  public:
    Engine();
    bool Init();
    void Start();
    void SetTitle(std::string title);
    void SetNetworkInfo(NetworkInfo network_info);
    NetworkInfo GetNetworkInfo();
    void SetBackgroundColor(Color color);
    void SetShowPlayerBorder(bool show_player_border);
    void SetShowZoneBorders(bool show_zone_borders);
    void SetPlayerTextures(int player_textures);
    void SetMaxPlayers(int max_players);
    void BaseTimelineChangeTic(double tic);
    double BaseTimelineGetTic();
    void BaseTimelineTogglePause();
    std::vector<Entity *> GetEntities();
    std::vector<Entity *> GetNetworkedEntities();
    void AddEntity(Entity *entity);
    void AddSideBoundary(Position position, Size size);
    Entity *GetSpawnPoint(int index);
    void AddSpawnPoint(Position position, Size size);
    void AddDeathZone(Position position, Size size);
    void SetCallback(std::function<void(std::vector<Entity *> *)> callback);
};