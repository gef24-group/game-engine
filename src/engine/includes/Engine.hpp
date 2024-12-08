#pragma once

#include "App.hpp"
#include "EngineHandler.hpp"
#include "Entity.hpp"
#include "Input.hpp"
#include "Timeline.hpp"
#include "Types.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <zmq.hpp>

extern App *app;

class Engine {
  public:
    static Engine &GetInstance() {
        static Engine instance;
        return instance;
    }

  private:
    Engine();

  public:
    Engine(Engine const &) = delete;
    void operator=(Engine const &) = delete;

  private:
    std::string title;
    std::shared_ptr<Timeline> engine_timeline;
    std::unique_ptr<Input> input;
    std::unique_ptr<EngineHandler> engine_handler;
    NetworkInfo network_info;
    Encoding encoding;
    std::atomic<int> players_connected;
    Color background_color;
    bool show_player_border;
    int player_textures;
    int max_players;

    std::shared_ptr<Entity> camera;
    bool show_zone_borders;
    Color side_boundary_color;
    Color spawn_point_color;
    Color death_zone_color;

    std::mutex entities_mutex;
    std::vector<Entity *> entities;
    std::unordered_map<Entity *, std::pair<Position, double>> entity_transforms;
    std::function<void(std::vector<Entity *> &)> callback;

    std::thread listener_thread;
    std::thread receive_broadcast_thread;
    std::vector<std::thread> client_threads;
    std::vector<std::thread> peer_threads;

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

    std::mutex broadcast_mutex;

    bool InitSingleClient();
    bool InitCSServer();
    bool InitCSClient();
    bool InitCSClientConnection();
    bool InitP2PHost();
    bool InitP2PPeer();
    bool InitP2PPeerConnection();

    void StartSingleClient();
    void StartCSServer();
    void StartCSClient();
    void StartP2P();

    bool InitializeDisplay();
    void ShowWelcomeScreen();

    void CSServerClientThread(int player_id);
    void CSServerListenerThread();
    void CSServerBroadcastPlayers();
    void CSClientReceiveBroadcastThread();

    Entity *CreateNewPlayer(int player_id, std::string player_address = "");
    Entity *GetSpawnPoint(int index);

    void P2PHostListenerThread();
    void P2PHostBroadcastPlayers();
    void P2PReceiveBroadcastFromPeerThread(int player_id, std::string player_address);
    void P2PReceiveBroadcastFromHostThread();

    void SendInactiveUpdate();

    void EncodeMessage(const EntityUpdate &entity_update, zmq::message_t &message);
    void DecodeMessage(const zmq::message_t &message, EntityUpdate &entity_update);

    bool HandleQuitEvent();
    void GetTimeDelta();
    void ApplyEntityPhysicsAndUpdates();
    void TestCollision();
    void ResetSideBoundaries();
    void SetSideBoundaryVelocities(Velocity velocity);
    void Update();
    void SetEntityTransforms();
    void RecordEvents();
    void HandleScaling();
    void RenderScene();
    void RenderBackground();
    void RenderBorder();
    void CaptureTracyFrameImage();
    void Shutdown();

  public:
    bool Init();
    void Start();
    void SetTitle(std::string title);
    void SetEncoding(Encoding encoding);
    void SetNetworkInfo(NetworkInfo network_info);
    NetworkInfo GetNetworkInfo();
    void SetBackgroundColor(Color color);
    void SetShowPlayerBorder(bool show_player_border);
    void SetShowZoneBorders(bool show_zone_borders);
    void ToggleShowZoneBorders();
    void SetPlayerTextures(int player_textures);
    void SetMaxPlayers(int max_players);
    void EngineTimelineChangeTic(double tic);
    double EngineTimelineGetTic();
    int64_t EngineTimelineGetTime();
    FrameTime EngineTimelineGetFrameTime();
    void EngineTimelineTogglePause();
    std::vector<Entity *> GetEntities();
    std::vector<Entity *> GetNetworkedEntities();
    void AddEntity(Entity *entity);
    void RemoveEntity(Entity *entity);
    void AddSideBoundary(Position position, Size size);
    void AddSpawnPoint(Position position, Size size);
    void AddDeathZone(Position position, Size size);
    void RespawnPlayer();
    void HandleSideBoundaries(Entity *side_boundary);
    void SetCallback(std::function<void(std::vector<Entity *> &)> callback);

    void BindPauseKey(SDL_Scancode key);
    void BindSpeedDownKey(SDL_Scancode key);
    void BindSpeedUpKey(SDL_Scancode key);
    void BindDisplayScalingKey(SDL_Scancode key);
    void BindHiddenZoneKey(SDL_Scancode key);
    void BindRecordKey(SDL_Scancode key);
    void BindReplayKey(SDL_Scancode key);

    void RegisterInputChord(int chord_id, std::unordered_set<SDL_Scancode> chord);

    void CSServerBroadcastUpdates(Entity *entity);
    void CSClientSendUpdate();
    void P2PBroadcastUpdates(Entity *entity);

    void OnJoin(std::string player_address);
    void OnDiscover();
    void OnLeave();
};