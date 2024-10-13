#include "Engine.hpp"
#include "Collision.hpp"
#include "Entity.hpp"
#include "Handler.hpp"
#include "Network.hpp"
#include "Physics.hpp"
#include "Render.hpp"
#include "SDL.h"
#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_keyboard.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_scancode.h"
#include "SDL_video.h"
#include "Timeline.hpp"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <chrono>
#include <climits>
#include <csignal>
#include <string>
#include <thread>
#include <vector>
#include <zmq.hpp>

Engine::Engine() {
    std::signal(SIGINT, HandleSIGINT);
    app->sdl_window = nullptr;
    app->renderer = nullptr;
    app->quit.store(false);
    app->sigint.store(false);
    app->key_map = new KeyMap();
    app->window = Window({1920, 1080, true});

    this->title = "";
    this->engine_timeline = std::make_shared<Timeline>();
    this->players_connected.store(0);
    this->background_color = Color{0, 0, 0, 255};
    this->show_player_border = false;
    this->show_zone_borders = false;
    this->player_textures = INT_MAX;
    this->max_players = INT_MAX;

    this->camera = std::make_shared<Entity>("camera", EntityCategory::Camera);
    this->camera->AddComponent<Transform>();
    this->camera->AddComponent<Physics>();
    this->camera->GetComponent<Physics>()->SetEngineTimeline(this->engine_timeline);

    this->callback = [](std::vector<Entity *> *) {};

    this->stop_input_thread.store(false);
    this->stop_listener_thread.store(false);
    this->stop_receive_broadcast_thread.store(false);
    this->stop_client_thread.store(false);
    this->stop_peer_thread.store(false);
}

bool Engine::Init() {
    if (this->network_info.mode == NetworkMode::Single &&
        this->network_info.role == NetworkRole::Client) {
        return this->InitSingleClient();
    }

    if (this->network_info.mode == NetworkMode::ClientServer) {
        if (this->network_info.role == NetworkRole::Server) {
            return this->InitCSServer();
        }
        if (this->network_info.role == NetworkRole::Client) {
            return this->InitCSClient();
        }
    }

    if (this->network_info.mode == NetworkMode::PeerToPeer) {
        if (this->network_info.role == NetworkRole::Host) {
            return this->InitP2PHost();
        }
        if (this->network_info.role == NetworkRole::Peer) {
            return this->InitP2PPeer();
        }
    }

    return false;
}

bool Engine::InitSingleClient() {
    bool display_success = this->InitializeDisplay();
    this->ShowWelcomeScreen();
    return display_success;
}

void Engine::CSServerClientThread(int player_id) {
    zmq::socket_t client_socket(this->zmq_context, zmq::socket_type::rep);
    client_socket.set(zmq::sockopt::linger, 0);
    client_socket.bind("tcp://*:600" + std::to_string(player_id));

    Log(LogLevel::Info, "Client thread for client [%d] started", player_id);

    while (!this->stop_client_thread.load()) {
        try {
            zmq::message_t request;
            zmq::recv_result_t result = client_socket.recv(request, zmq::recv_flags::none);
            EntityUpdate entity_update;
            std::memcpy(&entity_update, request.data(), sizeof(EntityUpdate));

            std::string ack = "Acknowledge client [" + std::to_string(player_id) + "]";
            zmq::message_t reply(ack.size());
            std::memcpy(reply.data(), ack.c_str(), ack.size());
            client_socket.send(reply, zmq::send_flags::none);

            Entity *entity = GetEntityByName(entity_update.name, this->GetEntities());
            if (entity) {
                if (!entity_update.active) {
                    entity->SetActive(false);
                } else {
                    entity->GetComponent<Transform>()->SetPosition(entity_update.position);
                }
            }

        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the server client thread: %s", e.what());
            client_socket.close();
            this->server_broadcast_socket.close();
        }
    }
};

void Engine::CSServerBroadcastUpdates() {
    for (Entity *entity : this->GetNetworkedEntities()) {
        try {
            EntityUpdate entity_update;
            std::snprintf(entity_update.name, sizeof(entity_update.name), "%s",
                          entity->GetName().c_str());
            entity_update.position = entity->GetComponent<Transform>()->GetPosition();
            if (!entity->GetActive()) {
                entity_update.active = false;
                Log(LogLevel::Info, "Client %s exiting", Split(entity_update.name, '_')[1].c_str());
                this->RemoveEntity(entity);
            }

            zmq::message_t broadcast_update(sizeof(EntityUpdate));
            std::memcpy(broadcast_update.data(), &entity_update, sizeof(EntityUpdate));
            this->server_broadcast_socket.send(broadcast_update, zmq::send_flags::none);
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error while broadcasting server updates: %s", e.what());
            this->server_broadcast_socket.close();
        }
    }
}

void Engine::CSServerListenerThread() {
    Log(LogLevel::Info, "Server listening for incoming connections at port 5555");

    while (!this->stop_listener_thread.load()) {
        try {
            zmq::message_t request;
            zmq::recv_result_t result = this->join_socket.recv(request, zmq::recv_flags::none);
            std::string message(static_cast<char *>(request.data()), request.size());

            // If the server receives a join message from the client, a new entity is created
            // for the player, and a thread meant solely for communication between the server and
            // that client is spawned
            if (message == "join") {
                int player_id = this->players_connected += 1;

                JoinReply join_reply;
                join_reply.player_id = player_id;

                zmq::message_t reply_msg(sizeof(JoinReply));
                std::memcpy(reply_msg.data(), &join_reply, sizeof(JoinReply));
                this->join_socket.send(reply_msg, zmq::send_flags::none);

                if (this->players_connected <= this->max_players) {
                    this->CreateNewPlayer(player_id);
                }

                this->client_threads.emplace_back(
                    [this, player_id]() { this->CSServerClientThread(player_id); });
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the server listener thread: %s", e.what());
            this->join_socket.close();
        }
    }

    this->stop_client_thread.store(true);
    for (auto &client_thread : this->client_threads) {
        if (client_thread.joinable()) {
            client_thread.join();
        }
    }
};

bool Engine::InitCSServer() {
    this->zmq_context = zmq::context_t(1);

    this->join_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::rep);
    this->join_socket.set(zmq::sockopt::linger, 0);
    this->join_socket.bind("tcp://*:5555");
    this->listener_thread = std::thread([this]() { this->CSServerListenerThread(); });

    this->server_broadcast_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::pub);
    this->server_broadcast_socket.set(zmq::sockopt::linger, 0);
    this->server_broadcast_socket.bind("tcp://*:5556");

    return true;
}

bool Engine::InitCSClientConnection() {
    try {
        this->zmq_context = zmq::context_t(1);
        int join_port = 5555;
        int client_update_port = 6000;
        int server_broadcast_port = 5556;

        this->join_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::req);
        this->join_socket.set(zmq::sockopt::linger, 0);
        this->join_socket.connect(GetConnectionAddress(this->network_info.server_ip, join_port));

        std::string join_message = "join";
        zmq::message_t connection_request(join_message.size());
        std::memcpy(connection_request.data(), join_message.c_str(), join_message.size());
        this->join_socket.send(connection_request, zmq::send_flags::none);

        zmq::message_t server_reply;
        zmq::recv_result_t res = this->join_socket.recv(server_reply, zmq::recv_flags::none);

        JoinReply join_reply;
        std::memcpy(&join_reply, server_reply.data(), sizeof(JoinReply));
        Log(LogLevel::Info, "The player ID assigned by the server: %s",
            std::to_string(join_reply.player_id).c_str());
        this->network_info.id = join_reply.player_id;

        this->client_update_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::req);
        this->client_update_socket.set(zmq::sockopt::linger, 0);
        this->client_update_socket.connect(GetConnectionAddress(
            this->network_info.server_ip, client_update_port + this->network_info.id));

        this->server_broadcast_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::sub);
        this->server_broadcast_socket.set(zmq::sockopt::linger, 0);
        this->server_broadcast_socket.connect(
            GetConnectionAddress(this->network_info.server_ip, server_broadcast_port));
        this->server_broadcast_socket.set(zmq::sockopt::subscribe, "");

        return true;
    } catch (const zmq::error_t &e) {
        Log(LogLevel::Info, "Caught error while initializing client connection: %s", e.what());
        this->join_socket.close();
        this->client_update_socket.close();
        this->server_broadcast_socket.close();

        return false;
    }
}

bool Engine::InitP2PPeerConnection() {
    try {
        this->zmq_context = zmq::context_t(1);
        int host_broadcast_port = 6001;
        int join_port = 5555;

        // The peer connects to the host peer's broadcast socket
        // One of the peers is called the 'host' since our design employs a listen-server
        // architecture
        this->host_broadcast_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::sub);
        this->host_broadcast_socket.set(zmq::sockopt::linger, 0);
        this->host_broadcast_socket.connect(
            GetConnectionAddress(this->network_info.host_ip, host_broadcast_port));
        this->host_broadcast_socket.set(zmq::sockopt::subscribe, "");

        // Connecting to the host in order to send a join message
        this->join_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::req);
        this->join_socket.set(zmq::sockopt::linger, 0);
        this->join_socket.connect(GetConnectionAddress(this->network_info.host_ip, join_port));

        std::string join_message = "join " + this->network_info.peer_ip;
        zmq::message_t connection_request(join_message.size());
        std::memcpy(connection_request.data(), join_message.c_str(), join_message.size());
        this->join_socket.send(connection_request, zmq::send_flags::none);

        zmq::message_t server_reply;
        zmq::recv_result_t res = this->join_socket.recv(server_reply, zmq::recv_flags::none);

        // From the host's reply, it gets its own client ID.
        JoinReply join_reply;
        std::memcpy(&join_reply, server_reply.data(), sizeof(JoinReply));
        Log(LogLevel::Info, "The player ID assigned by the host: %s",
            std::to_string(join_reply.player_id).c_str());
        this->network_info.id = join_reply.player_id;

        // The peer acts as a server here that broadcasts its own position to all the peers
        this->peer_broadcast_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::pub);
        this->peer_broadcast_socket.set(zmq::sockopt::linger, 0);
        this->peer_broadcast_socket.bind("tcp://*:600" + std::to_string(this->network_info.id));

        return true;
    } catch (const zmq::error_t &e) {
        Log(LogLevel::Info, "Caught error while initializing peer connection: %s", e.what());
        this->host_broadcast_socket.close();
        this->join_socket.close();
        this->peer_broadcast_socket.close();

        return false;
    }
}

bool Engine::InitCSClient() {
    bool display_success = this->InitializeDisplay();
    bool client_connection_success = this->InitCSClientConnection();
    this->ShowWelcomeScreen();

    return display_success && client_connection_success;
}

void Engine::P2PHostBroadcastPlayers() {
    for (Entity *entity : this->GetNetworkedEntities()) {
        try {
            if (entity->GetComponent<Network>()->GetOwner() == Peer) {
                EntityUpdate entity_update;
                std::snprintf(entity_update.name, sizeof(entity_update.name), "%s",
                              entity->GetName().c_str());
                entity_update.position = entity->GetComponent<Transform>()->GetPosition();
                std::snprintf(entity_update.player_address, sizeof(entity_update.player_address),
                              "%s", entity->GetComponent<Network>()->GetPlayerAddress().c_str());

                zmq::message_t broadcast_update(sizeof(EntityUpdate));
                std::memcpy(broadcast_update.data(), &entity_update, sizeof(EntityUpdate));
                this->host_broadcast_socket.send(broadcast_update, zmq::send_flags::none);
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error while broadcasting host players: %s", e.what());
            this->host_broadcast_socket.close();
        }
    }
}

void Engine::P2PHostListenerThread() {
    Log(LogLevel::Info, "Host listening for incoming connections at port 5555");

    while (!this->stop_listener_thread.load()) {
        try {
            zmq::message_t request;
            zmq::recv_result_t result = this->join_socket.recv(request, zmq::recv_flags::none);
            std::string message(static_cast<char *>(request.data()), request.size());

            // when a join message is received, the listen-server creates a new entity for that
            // player. It also spawns a new thread dedicated to receiving broadcasts from that peer
            if (Split(message, ' ')[0] == "join") {
                int player_id = this->players_connected += 1;
                std::string player_address = Split(message, ' ')[1];

                JoinReply join_reply;
                join_reply.player_id = player_id;

                zmq::message_t reply_msg(sizeof(JoinReply));
                std::memcpy(reply_msg.data(), &join_reply, sizeof(JoinReply));
                this->join_socket.send(reply_msg, zmq::send_flags::none);

                if (this->players_connected <= this->max_players) {
                    this->CreateNewPlayer(player_id, player_address);
                }

                this->peer_threads.emplace_back([this, player_id, player_address]() {
                    this->P2PReceiveBroadcastFromPeerThread(player_id, player_address);
                });
            }

            if (message == "discover") {
                std::string ack = "Acknowledge discover";
                zmq::message_t reply(ack.size());
                std::memcpy(reply.data(), ack.c_str(), ack.size());
                this->join_socket.send(reply, zmq::send_flags::none);

                // Whenever a new peer joins the game, the discover request broadcasts the status of
                // all peers the host knows about, so that all peers are aware of how many players
                // are playing the game at the time.
                this->P2PHostBroadcastPlayers();
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the host listener thread: %s", e.what());
            // app->quit.store(true);
            this->join_socket.close();
        }
    }

    this->stop_peer_thread.store(true);
    for (auto &peer_thread : this->peer_threads) {
        if (peer_thread.joinable()) {
            peer_thread.join();
        }
    }
};

bool Engine::InitP2PHost() {
    bool display_success = this->InitializeDisplay();

    this->zmq_context = zmq::context_t(1);

    this->join_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::rep);
    this->join_socket.set(zmq::sockopt::linger, 0);
    this->join_socket.bind("tcp://*:5555");
    this->listener_thread = std::thread([this]() { this->P2PHostListenerThread(); });

    this->host_broadcast_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::pub);
    this->host_broadcast_socket.set(zmq::sockopt::linger, 0);
    this->network_info.id = 1;
    this->players_connected.store(1);
    this->host_broadcast_socket.bind("tcp://*:600" +
                                     std::to_string(this->players_connected.load()));

    this->ShowWelcomeScreen();

    return display_success;
}

bool Engine::InitP2PPeer() {
    bool display_success = this->InitializeDisplay();
    bool connection_success = this->InitP2PPeerConnection();
    this->ShowWelcomeScreen();

    return display_success && connection_success;
}

void Engine::Start() {
    if (this->network_info.mode == NetworkMode::Single &&
        this->network_info.role == NetworkRole::Client) {
        this->StartSingleClient();
    }

    if (this->network_info.mode == NetworkMode::ClientServer) {
        if (this->network_info.role == NetworkRole::Server) {
            this->StartCSServer();
        }
        if (this->network_info.role == NetworkRole::Client) {
            this->StartCSClient();
        }
    }

    if (this->network_info.mode == NetworkMode::PeerToPeer) {
        this->StartP2P();
    }
}

void Engine::StartSingleClient() {

    this->input_thread = std::thread([this]() { this->ReadInputsThread(); });

    this->engine_timeline->SetFrameTime(FrameTime{0, this->engine_timeline->GetTime(), 0});

    // Engine loop
    while (!app->quit.load() && !app->sigint.load()) {
        app->quit.store(this->HandleQuitEvent());
        this->GetTimeDelta();
        this->ApplyEntityPhysicsAndUpdates();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
        this->RenderScene();
    }

    this->stop_input_thread.store(true);
    if (this->input_thread.joinable()) {
        this->input_thread.join();
    }

    this->Shutdown();
}

void Engine::StartCSServer() {
    this->engine_timeline->SetFrameTime(FrameTime{0, this->engine_timeline->GetTime(), 0});

    while (!app->sigint.load()) {
        this->GetTimeDelta();
        this->ApplyEntityPhysicsAndUpdates();
        this->CSServerBroadcastUpdates();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
    }

    this->zmq_context.shutdown();

    this->stop_listener_thread.store(true);
    if (this->listener_thread.joinable()) {
        this->listener_thread.join();
    }
}

void Engine::CSClientAddExistingPlayers() {
    for (int player_id = 1; player_id <= this->network_info.id; player_id++) {
        this->CreateNewPlayer(player_id);
    }
}

Entity *Engine::CreateNewPlayer(int player_id, std::string player_address) {
    bool is_p2p = this->network_info.mode == NetworkMode::PeerToPeer;
    bool is_host = this->network_info.role == NetworkRole::Host;
    bool is_cs = this->network_info.mode == NetworkMode::ClientServer;
    bool is_server = this->network_info.role == NetworkRole::Server;
    bool is_client = this->network_info.role == NetworkRole::Client;

    Entity *controllable = GetControllable(this->GetEntities());
    std::string player_name = Split(controllable->GetName(), '_')[0];
    player_name += "_" + std::to_string(player_id);

    if (is_p2p && is_host) {
        if (player_id == 1) {
            controllable->GetComponent<Network>()->SetOwner(NetworkRole::Host);
        }
    }
    if ((is_cs && is_server && player_id == 1) || (player_id == this->network_info.id)) {
        controllable->SetName(player_name);
        SetPlayerTexture(controllable, player_id, this->player_textures);
        if (this->show_player_border) {
            controllable->GetComponent<Render>()->SetBorder(Border{true, Color{0, 0, 0, 255}});
        }
        return controllable;
    }
    if ((is_cs && is_server && player_id > 1) || (player_id != this->network_info.id)) {
        Entity *player = new Entity(player_name, controllable->GetCategory());
        player->AddComponent<Render>();
        player->AddComponent<Transform>();
        player->AddComponent<Handler>();
        player->AddComponent<Physics>();
        player->AddComponent<Network>();
        player->AddComponent<Collision>();

        player->GetComponent<Render>()->SetColor(controllable->GetComponent<Render>()->GetColor());
        player->GetComponent<Transform>()->SetSize(
            controllable->GetComponent<Transform>()->GetSize());
        player->GetComponent<Render>()->SetTextureTemplate(
            controllable->GetComponent<Render>()->GetTextureTemplate());
        player->GetComponent<Handler>()->SetCallback(
            controllable->GetComponent<Handler>()->GetCallback());
        player->GetComponent<Network>()->SetOwner(
            controllable->GetComponent<Network>()->GetOwner());

        player->GetComponent<Physics>()->SetEngineTimeline(this->engine_timeline);
        player->GetComponent<Render>()->SetCamera(this->camera);

        if (is_p2p) {
            player->GetComponent<Network>()->SetOwner(NetworkRole::Peer);
            if (!player_address.empty()) {
                player->GetComponent<Network>()->SetPlayerAddress(player_address);
            }
        }
        SetPlayerTexture(player, player_id, this->player_textures);

        this->AddEntity(player);
        return player;
    }

    return nullptr;
}

void Engine::CSClientReceiveBroadcastThread() {
    while (!this->stop_receive_broadcast_thread.load()) {
        try {
            zmq::message_t message;
            zmq::recv_result_t res =
                this->server_broadcast_socket.recv(message, zmq::recv_flags::none);

            if (res) {
                EntityUpdate entity_update;
                std::memcpy(&entity_update, message.data(), sizeof(EntityUpdate));
                Entity *entity = GetEntityByName(entity_update.name, this->GetEntities());
                // If the entity received does not already exist in the client, create it. Occurs
                // whenever a new client joins the game
                if (entity == nullptr) {
                    int player_id = GetPlayerIdFromName(entity_update.name);
                    entity = this->CreateNewPlayer(player_id);
                }
                Entity *player = GetClientPlayer(this->network_info.id, this->GetEntities());

                if (entity->GetName() != player->GetName()) {
                    if (entity_update.active) {
                        entity->GetComponent<Transform>()->SetPosition(entity_update.position);
                    } else {
                        this->RemoveEntity(entity);
                    }
                }
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the client receive broadcast thread: %s",
                e.what());
            this->server_broadcast_socket.close();
        }
    }
}

void Engine::CSClientSendUpdate() {
    try {
        Entity *player = GetClientPlayer(this->network_info.id, this->GetEntities());
        EntityUpdate entity_update;
        std::snprintf(entity_update.name, sizeof(entity_update.name), "%s",
                      player->GetName().c_str());
        entity_update.position = player->GetComponent<Transform>()->GetPosition();
        if (app->quit.load()) {
            entity_update.active = false;
        }

        zmq::message_t update(sizeof(EntityUpdate));
        std::memcpy(update.data(), &entity_update, sizeof(EntityUpdate));
        this->client_update_socket.send(update, zmq::send_flags::none);

        zmq::message_t server_ack;
        zmq::recv_result_t res = this->client_update_socket.recv(server_ack, zmq::recv_flags::none);

        std::string server_ack_message(static_cast<const char *>(server_ack.data()),
                                       server_ack.size());
    } catch (const zmq::error_t &e) {
        Log(LogLevel::Info, "Caught error while sending client update: %s", e.what());
        this->client_update_socket.close();
    }
}

void Engine::StartCSClient() {
    this->CSClientAddExistingPlayers();

    this->input_thread = std::thread([this]() { this->ReadInputsThread(); });

    this->receive_broadcast_thread =
        std::thread([this]() { this->CSClientReceiveBroadcastThread(); });

    this->engine_timeline->SetFrameTime(FrameTime{0, this->engine_timeline->GetTime(), 0});

    // Engine loop
    while (!app->quit.load() && !app->sigint.load()) {
        app->quit.store(this->HandleQuitEvent());
        this->GetTimeDelta();
        this->ApplyEntityPhysicsAndUpdates();
        this->CSClientSendUpdate();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
        this->RenderScene();
    }

    this->zmq_context.shutdown();

    this->stop_input_thread.store(true);
    if (this->input_thread.joinable()) {
        this->input_thread.join();
    }

    this->stop_receive_broadcast_thread.store(true);
    if (this->receive_broadcast_thread.joinable()) {
        this->receive_broadcast_thread.join();
    }

    this->Shutdown();
}

void Engine::P2PBroadcastUpdates() {
    for (Entity *entity : GetEntitiesByRole(this->network_info, this->GetEntities())) {
        try {
            EntityUpdate entity_update;
            std::snprintf(entity_update.name, sizeof(entity_update.name), "%s",
                          entity->GetName().c_str());
            entity_update.position = entity->GetComponent<Transform>()->GetPosition();
            if (app->quit.load()) {
                entity_update.active = false;
            }

            zmq::message_t broadcast_update(sizeof(EntityUpdate));
            std::memcpy(broadcast_update.data(), &entity_update, sizeof(EntityUpdate));

            if (this->network_info.role == Host) {
                // The host peer broadcasts the positions of all host governed entities
                this->host_broadcast_socket.send(broadcast_update, zmq::send_flags::none);

            } else if (this->network_info.role == Peer &&
                       this->network_info.id == GetPlayerIdFromName(entity->GetName())) {
                // The other peers broadcast the position of its own controllable player
                this->peer_broadcast_socket.send(broadcast_update, zmq::send_flags::none);
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error while broadcasting p2p updates: %s", e.what());
            if (this->network_info.role == Host) {
                this->host_broadcast_socket.close();
            } else if (this->network_info.role == Peer) {
                this->peer_broadcast_socket.close();
            }
        }
    }
}

void Engine::P2PReceiveBroadcastFromPeerThread(int player_id, std::string player_address) {
    int peer_receive_port = 6000;
    std::string peer_receive_address =
        GetConnectionAddress(player_address, peer_receive_port + player_id);
    Log(LogLevel::Info, "Receiving broadcasts from player: [%s]", peer_receive_address.c_str());

    zmq::socket_t peer_receive_socket(this->zmq_context, zmq::socket_type::sub);
    peer_receive_socket.set(zmq::sockopt::linger, 0);
    peer_receive_socket.connect(peer_receive_address);
    peer_receive_socket.set(zmq::sockopt::subscribe, "");

    while (!this->stop_peer_thread.load()) {
        try {
            zmq::message_t message;
            zmq::recv_result_t res = peer_receive_socket.recv(message, zmq::recv_flags::none);

            if (res) {
                EntityUpdate entity_update;
                std::memcpy(&entity_update, message.data(), sizeof(EntityUpdate));
                Entity *entity = GetEntityByName(entity_update.name, this->GetEntities());
                if (entity_update.active) {
                    entity->GetComponent<Transform>()->SetPosition(entity_update.position);
                } else {
                    this->RemoveEntity(entity);
                }
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the peer receive broadcast thread: %s", e.what());
            peer_receive_socket.close();
        }
    }
}

void Engine::P2PReceiveBroadcastFromHostThread() {
    Log(LogLevel::Info, "Receiving broadcasts from the host");

    try {
        std::string discover_message = "discover";
        zmq::message_t discover_request(discover_message.size());
        std::memcpy(discover_request.data(), discover_message.c_str(), discover_message.size());
        this->join_socket.send(discover_request, zmq::send_flags::none);

        zmq::message_t server_reply;
        zmq::recv_result_t res = this->join_socket.recv(server_reply, zmq::recv_flags::none);
    } catch (const zmq::error_t &e) {
        Log(LogLevel::Info, "Caught error while discovering peers: %s", e.what());
        this->join_socket.close();
    }

    while (!this->stop_receive_broadcast_thread.load()) {
        try {
            zmq::message_t message;
            zmq::recv_result_t res =
                this->host_broadcast_socket.recv(message, zmq::recv_flags::none);

            if (res) {
                EntityUpdate entity_update;
                std::memcpy(&entity_update, message.data(), sizeof(EntityUpdate));
                Entity *entity = GetEntityByName(entity_update.name, this->GetEntities());
                // If the entity received does not already exist in the client, create it. Occurs
                // whenever a new client joins the game
                if (entity == nullptr) {
                    int player_id = GetPlayerIdFromName(entity_update.name);
                    std::string player_address = entity_update.player_address;
                    entity = this->CreateNewPlayer(player_id);
                    // spawn a new thread to receive broadcasts from the new peer
                    if (player_id != 1) {
                        this->peer_threads.emplace_back([this, player_id, player_address]() {
                            this->P2PReceiveBroadcastFromPeerThread(player_id, player_address);
                        });
                    }
                }
                Entity *player = GetClientPlayer(this->network_info.id, this->GetEntities());

                if (entity->GetName() != player->GetName()) {
                    if (entity_update.active) {
                        entity->GetComponent<Transform>()->SetPosition(entity_update.position);
                    } else {
                        this->RemoveEntity(entity);
                    }
                }
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the peer receive broadcast from host thread: %s",
                e.what());
            this->host_broadcast_socket.close();
        }
    }

    this->stop_peer_thread.store(true);
    for (auto &peer_thread : this->peer_threads) {
        if (peer_thread.joinable()) {
            peer_thread.join();
        }
    }
}

void Engine::StartP2P() {

    this->input_thread = std::thread([this]() { this->ReadInputsThread(); });

    this->CreateNewPlayer(this->network_info.id);

    if (this->network_info.role == NetworkRole::Peer) {
        this->receive_broadcast_thread =
            std::thread([this]() { this->P2PReceiveBroadcastFromHostThread(); });
    }

    this->engine_timeline->SetFrameTime(FrameTime{0, this->engine_timeline->GetTime(), 0});

    // Engine loop
    while (!app->quit.load() && !app->sigint.load()) {
        app->quit.store(this->HandleQuitEvent());
        this->GetTimeDelta();
        this->ApplyEntityPhysicsAndUpdates();
        this->P2PBroadcastUpdates();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
        this->RenderScene();
    }

    this->zmq_context.shutdown();

    if (this->network_info.role == NetworkRole::Host) {
        this->stop_listener_thread.store(true);
        if (this->listener_thread.joinable()) {
            this->listener_thread.join();
        }
    }

    this->stop_input_thread.store(true);
    if (this->input_thread.joinable()) {
        this->input_thread.join();
    }

    if (this->network_info.role == NetworkRole::Peer) {
        this->stop_receive_broadcast_thread.store(true);
        if (this->receive_broadcast_thread.joinable()) {
            this->receive_broadcast_thread.join();
        }
    }

    this->Shutdown();
}

bool Engine::InitializeDisplay() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        Log(LogLevel::Error, "SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    SDL_Window *window = SDL_CreateWindow(
        this->title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, app->window.width,
        app->window.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        Log(LogLevel::Error, "SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return false;
    } else {
        app->sdl_window = window;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        SDL_DestroyWindow(window);
        Log(LogLevel::Error, "SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return false;
    } else {
        app->renderer = renderer;
    }

    return true;
}

void Engine::SetTitle(std::string title) {
    if (this->network_info.role == NetworkRole::Client ||
        this->network_info.role == NetworkRole::Host ||
        this->network_info.role == NetworkRole::Peer) {
        this->title = title;
    }
}

void Engine::SetNetworkInfo(NetworkInfo network_info) { this->network_info = network_info; }

NetworkInfo Engine::GetNetworkInfo() { return this->network_info; }

void Engine::BaseTimelineChangeTic(double tic) { this->engine_timeline->ChangeTic(tic); }

double Engine::BaseTimelineGetTic() { return this->engine_timeline->GetTic(); }

void Engine::BaseTimelineTogglePause() {
    this->engine_timeline->TogglePause(this->engine_timeline->GetFrameTime().current);
}

void Engine::SetBackgroundColor(Color color) {
    if (this->network_info.role == NetworkRole::Client ||
        this->network_info.role == NetworkRole::Host ||
        this->network_info.role == NetworkRole::Peer) {
        this->background_color = color;
    }
}

void Engine::SetShowPlayerBorder(bool show_player_border) {
    this->show_player_border = show_player_border;
}

void Engine::SetShowZoneBorders(bool show_zone_borders) {
    this->show_zone_borders = show_zone_borders;
}

void Engine::SetPlayerTextures(int player_textures) { this->player_textures = player_textures; }

void Engine::SetMaxPlayers(int max_players) { this->max_players = max_players; }

void Engine::ShowWelcomeScreen() {
    // Sets the background to blue
    SDL_SetRenderDrawColor(app->renderer, this->background_color.red, this->background_color.green,
                           this->background_color.blue, 255);
    // Clears the renderer
    SDL_RenderClear(app->renderer);
    SDL_RenderPresent(app->renderer);
}

std::vector<Entity *> Engine::GetEntities() {
    std::lock_guard<std::mutex> lock(this->entities_mutex);
    return this->entities;
}

std::vector<Entity *> Engine::GetNetworkedEntities() {
    std::vector<Entity *> networked_entities;
    for (Entity *entity : this->GetEntities()) {
        if (entity->GetComponent<Network>() != nullptr) {
            networked_entities.push_back(entity);
        }
    }
    return networked_entities;
}

void Engine::AddEntity(Entity *entity) {
    if (entity->GetComponent<Physics>() != nullptr) {
        entity->GetComponent<Physics>()->SetEngineTimeline(this->engine_timeline);
    }
    if (entity->GetComponent<Render>() != nullptr) {
        entity->GetComponent<Render>()->SetCamera(this->camera);
    }
    if (entity->GetCategory() == EntityCategory::Controllable) {
        entity->GetComponent<Transform>()->SetPosition(
            GetSpawnPoint(this->network_info.id - 1)->GetComponent<Transform>()->GetPosition());
    }

    std::lock_guard<std::mutex> lock(this->entities_mutex);
    this->entities.push_back(entity);
}

void Engine::AddSideBoundary(Position position, Size size) {
    size_t side_boundary_index =
        GetEntitiesByCategory(this->GetEntities(), EntityCategory::SideBoundary).size();
    Entity *side_boundary = new Entity("side_boundary_" + std::to_string(side_boundary_index),
                                       EntityCategory::SideBoundary);
    side_boundary->AddComponent<Transform>();
    side_boundary->AddComponent<Physics>();
    side_boundary->GetComponent<Transform>()->SetPosition(position);
    side_boundary->GetComponent<Transform>()->SetSize(size);

    if (this->show_zone_borders) {
        side_boundary->AddComponent<Render>();
        side_boundary->GetComponent<Render>()->SetBorder(Border{true, Color{0, 0, 255, 255}});
    }

    this->AddEntity(side_boundary);
}

void Engine::RemoveEntity(Entity *entity) {
    std::lock_guard<std::mutex> lock(this->entities_mutex);
    auto iterator = std::find(entities.begin(), entities.end(), entity);

    if (iterator != entities.end()) {
        entities.erase(iterator);
        ;
    }
}

Entity *Engine::GetSpawnPoint(int index) {
    std::vector<Entity *> spawn_points =
        GetEntitiesByCategory(this->GetEntities(), EntityCategory::SpawnPoint);

    if (spawn_points.size() == 0) {
        return nullptr;
    }

    if (index >= 0 && index < spawn_points.size()) {
        return spawn_points[index];
    }

    int random_index = GetRandomInt(int(spawn_points.size() - 1));
    return spawn_points[random_index];
}

void Engine::AddSpawnPoint(Position position, Size size) {
    size_t spawn_point_index =
        GetEntitiesByCategory(this->GetEntities(), EntityCategory::SpawnPoint).size();
    Entity *spawn_point =
        new Entity("spawn_point_" + std::to_string(spawn_point_index), EntityCategory::SpawnPoint);
    spawn_point->AddComponent<Transform>();
    spawn_point->GetComponent<Transform>()->SetPosition(position);
    spawn_point->GetComponent<Transform>()->SetSize(size);

    if (this->show_zone_borders) {
        spawn_point->AddComponent<Render>();
        spawn_point->GetComponent<Render>()->SetBorder(Border{true, Color{0, 255, 0, 255}});
    }

    this->AddEntity(spawn_point);
}

void Engine::AddDeathZone(Position position, Size size) {
    size_t death_zone_index =
        GetEntitiesByCategory(this->GetEntities(), EntityCategory::DeathZone).size();
    Entity *death_zone =
        new Entity("death_zone_" + std::to_string(death_zone_index), EntityCategory::DeathZone);
    death_zone->AddComponent<Transform>();
    death_zone->GetComponent<Transform>()->SetPosition(position);
    death_zone->GetComponent<Transform>()->SetSize(size);

    if (this->show_zone_borders) {
        death_zone->AddComponent<Render>();
        death_zone->GetComponent<Render>()->SetBorder(Border{true, Color{255, 0, 0, 255}});
    }

    this->AddEntity(death_zone);
}

void Engine::SetCallback(std::function<void(std::vector<Entity *> *)> callback) {
    this->callback = callback;
}

void Engine::Update() {
    std::vector<Entity *> entities = this->GetEntities();
    this->callback(&entities);
}

void Engine::GetTimeDelta() {
    int64_t current = this->engine_timeline->GetTime();
    int64_t last = this->engine_timeline->GetFrameTime().last;
    int64_t delta = current - last;
    last = current;
    delta = std::clamp(delta, static_cast<int64_t>(0),
                       static_cast<int64_t>(16'000'000 / this->engine_timeline->GetTic()));

    this->engine_timeline->SetFrameTime(FrameTime{current, last, delta});
}

void Engine::ReadInputsThread() {
    while (!this->stop_input_thread.load()) {
        const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
        auto now = std::chrono::high_resolution_clock::now();

        auto debounce_key = [keyboard_state, now](int scancode, Key &key, bool delay) {
            if (!delay) {
                key.pressed.store(keyboard_state[scancode] != 0);
                return;
            }

            if (keyboard_state[scancode] != 0) {
                auto press_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - key.last_pressed_time);
                if (press_duration.count() > 50 && !key.pressed.load()) {
                    key.pressed.store(true);
                    key.OnPress();
                } else {
                    key.pressed.store(false);
                    key.last_pressed_time = now;
                }
            } else {
                key.pressed.store(false);
            }
        };

        debounce_key(SDL_SCANCODE_X, app->key_map->key_X, true);
        debounce_key(SDL_SCANCODE_P, app->key_map->key_P, true);
        debounce_key(SDL_SCANCODE_COMMA, app->key_map->key_comma, true);
        debounce_key(SDL_SCANCODE_PERIOD, app->key_map->key_period, true);

        debounce_key(SDL_SCANCODE_W, app->key_map->key_W, false);
        debounce_key(SDL_SCANCODE_A, app->key_map->key_A, false);
        debounce_key(SDL_SCANCODE_S, app->key_map->key_S, false);
        debounce_key(SDL_SCANCODE_D, app->key_map->key_D, false);

        debounce_key(SDL_SCANCODE_UP, app->key_map->key_up, false);
        debounce_key(SDL_SCANCODE_LEFT, app->key_map->key_left, false);
        debounce_key(SDL_SCANCODE_DOWN, app->key_map->key_down, false);
        debounce_key(SDL_SCANCODE_RIGHT, app->key_map->key_right, false);

        debounce_key(SDL_SCANCODE_SPACE, app->key_map->key_space, false);
    }
}

void Engine::ApplyEntityPhysicsAndUpdates() {
    std::vector<Entity *> entities = GetEntitiesByRole(this->network_info, this->GetEntities());

    for (Entity *entity : entities) {
        if (entity->GetComponent<Physics>() != nullptr) {
            entity->GetComponent<Physics>()->Update();
        }
        if (entity->GetComponent<Handler>() != nullptr) {
            entity->GetComponent<Handler>()->Update();
        }
    }
}

void Engine::TestCollision() {
    std::vector<Entity *> entities = this->GetEntities();

    for (int i = 0; i < entities.size() - 1; i++) {
        for (int j = i + 1; j < entities.size(); j++) {
            SDL_Rect entity_1 = {static_cast<int>(std::round(
                                     entities[i]->GetComponent<Transform>()->GetPosition().x)),
                                 static_cast<int>(std::round(
                                     entities[i]->GetComponent<Transform>()->GetPosition().y)),
                                 entities[i]->GetComponent<Transform>()->GetSize().width,
                                 entities[i]->GetComponent<Transform>()->GetSize().height};
            SDL_Rect entity_2 = {static_cast<int>(std::round(
                                     entities[j]->GetComponent<Transform>()->GetPosition().x)),
                                 static_cast<int>(std::round(
                                     entities[j]->GetComponent<Transform>()->GetPosition().y)),
                                 entities[j]->GetComponent<Transform>()->GetSize().width,
                                 entities[j]->GetComponent<Transform>()->GetSize().height};

            if (SDL_HasIntersection(&entity_1, &entity_2)) {
                if (entities[i]->GetComponent<Collision>() != nullptr) {
                    entities[i]->GetComponent<Collision>()->AddCollider(entities[j]);
                }
                if (entities[j]->GetComponent<Collision>() != nullptr) {
                    entities[j]->GetComponent<Collision>()->AddCollider(entities[i]);
                }
            } else {
                if (entities[i]->GetComponent<Collision>() != nullptr) {
                    entities[i]->GetComponent<Collision>()->RemoveCollider(entities[j]);
                }
                if (entities[j]->GetComponent<Collision>() != nullptr) {
                    entities[j]->GetComponent<Collision>()->RemoveCollider(entities[i]);
                }
            }
        }
    }
}

void Engine::HandleCollisions() {
    // TODO: Look into multithreading this later
    bool collided_with_death_zone = this->HandleDeathZones();
    if (!collided_with_death_zone) {
        this->HandleSideBoundaries();
    }
    for (Entity *entity : GetEntitiesByRole(this->network_info, this->GetEntities())) {
        if (entity->GetComponent<Collision>() != nullptr) {
            entity->GetComponent<Collision>()->Update();
        }
    }
}

bool Engine::HandleDeathZones() {
    Entity *player = GetClientPlayer(this->network_info.id, this->GetEntities());
    if (player == nullptr) {
        return false;
    }

    for (Entity *collider : player->GetComponent<Collision>()->GetColliders()) {
        if (collider->GetCategory() == EntityCategory::DeathZone) {
            Log(LogLevel::Info, "Collided with a death zone");
            Entity *respawn_point = this->GetSpawnPoint(this->network_info.id - 1);
            this->camera->GetComponent<Transform>()->SetPosition(Position{0, 0});
            player->GetComponent<Transform>()->SetPosition(
                respawn_point->GetComponent<Transform>()->GetPosition());
            this->ResetSideBoundaries();
            return true;
        }
    }
    return false;
}

void Engine::HandleSideBoundaries() {
    this->SetSideBoundaryVelocities(Velocity{0, 0});
    this->camera->GetComponent<Physics>()->SetVelocity({0, 0});

    Entity *player = GetClientPlayer(this->network_info.id, this->GetEntities());
    if (player == nullptr) {
        return;
    }

    for (Entity *collider : player->GetComponent<Collision>()->GetColliders()) {

        if (collider->GetCategory() == EntityCategory::SideBoundary) {
            Log(LogLevel::Info, "collision with a side boundary");
            int obj_x =
                static_cast<int>(std::round(player->GetComponent<Transform>()->GetPosition().x));
            int obj_y =
                static_cast<int>(std::round(player->GetComponent<Transform>()->GetPosition().y));
            int col_x =
                static_cast<int>(std::round(collider->GetComponent<Transform>()->GetPosition().x));
            int col_y =
                static_cast<int>(std::round(collider->GetComponent<Transform>()->GetPosition().y));

            int obj_width = player->GetComponent<Transform>()->GetSize().width;
            int obj_height = player->GetComponent<Transform>()->GetSize().height;
            int col_width = collider->GetComponent<Transform>()->GetSize().width;
            int col_height = collider->GetComponent<Transform>()->GetSize().height;

            SDL_Rect rect_1 = {obj_x, obj_y, obj_width, obj_height};
            SDL_Rect rect_2 = {col_x, col_y, col_width, col_height};
            Overlap overlap = GetOverlap(rect_1, rect_2);
            float vel_x = player->GetComponent<Physics>()->GetVelocity().x;
            float vel_y = player->GetComponent<Physics>()->GetVelocity().y;

            if (overlap == Overlap::Left || overlap == Overlap::Right) {
                this->camera->GetComponent<Physics>()->SetVelocity({vel_x, 0});
                Log(LogLevel::Info, "Setting side-boundary vel %f", vel_x);
                this->SetSideBoundaryVelocities({vel_x, 0});
            }
            if (overlap == Overlap::Top || overlap == Overlap::Bottom) {
                this->camera->GetComponent<Physics>()->SetVelocity({0, vel_y});
                this->SetSideBoundaryVelocities({0, vel_y});
            }
        }
    }

    this->camera->GetComponent<Physics>()->Update();

    for (Entity *entity : this->GetEntities()) {
        if (entity->GetCategory() == EntityCategory::SideBoundary) {
            entity->GetComponent<Physics>()->Update();
        }
    }
}

void Engine::ResetSideBoundaries() {
    std::vector<Entity *> side_boundaries =
        GetEntitiesByCategory(this->GetEntities(), EntityCategory::SideBoundary);
    for (Entity *side_boundary : side_boundaries) {
        // The screen position of the side boundary will always be equal to its original position
        Position original_position = side_boundary->GetComponent<Render>()->GetScreenPosition();
        side_boundary->GetComponent<Physics>()->SetVelocity(Velocity{0, 0});
        side_boundary->GetComponent<Transform>()->SetPosition(original_position);
    }
}

void Engine::SetSideBoundaryVelocities(Velocity velocity) {
    std::vector<Entity *> side_boundaries =
        GetEntitiesByCategory(this->GetEntities(), EntityCategory::SideBoundary);
    for (Entity *side_boundary : side_boundaries) {
        side_boundary->GetComponent<Physics>()->SetVelocity(velocity);
    }
}

bool Engine::HandleQuitEvent() {
    SDL_Event event;
    bool quit = false;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
    }
    return quit;
}

void Engine::RenderScene() {
    this->HandleScaling();

    this->RenderBackground();
    for (Entity *entity : this->GetEntities()) {
        if (entity->GetComponent<Render>() != nullptr) {
            entity->GetComponent<Render>()->Update();
        }
    }
    SDL_RenderPresent(app->renderer);
}

void Engine::RenderBackground() {
    // Add conditions to change the background later
    // Add options to render an image as a background later
    SDL_SetRenderDrawColor(app->renderer, this->background_color.red, this->background_color.green,
                           this->background_color.blue, 255);
    SDL_RenderClear(app->renderer);
}

void Engine::HandleScaling() {
    int set_logical_size_err;

    if (app->window.proportional_scaling) {
        set_logical_size_err =
            SDL_RenderSetLogicalSize(app->renderer, app->window.width, app->window.height);
    } else {
        set_logical_size_err = SDL_RenderSetLogicalSize(app->renderer, 0, 0);
    }

    if (set_logical_size_err) {
        Log(LogLevel::Error, "Set Viewport failed: %s", SDL_GetError());
    }
}

void Engine::Shutdown() {
    this->join_socket.close();
    this->server_broadcast_socket.close();
    this->client_update_socket.close();
    this->peer_broadcast_socket.close();
    this->host_broadcast_socket.close();
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->sdl_window);
    SDL_Quit();
    delete app->key_map;
    delete app;
}