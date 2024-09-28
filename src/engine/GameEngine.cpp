#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "SDL.h"
#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_keyboard.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_scancode.h"
#include "SDL_video.h"
#include "Timeline.hpp"
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

GameEngine::GameEngine() {
    std::signal(SIGINT, HandleSIGINT);
    app->sdl_window = nullptr;
    app->renderer = nullptr;
    app->quit.store(false);
    app->sigint.store(false);
    app->key_map = new KeyMap();
    app->window = Window({1920, 1080, true});

    this->game_title = "";
    this->engine_timeline = Timeline();
    this->players_connected.store(0);
    this->background_color = Color{0, 0, 0, 255};
    this->show_player_border = false;
    this->player_textures = INT_MAX;
    this->max_players = INT_MAX;
    this->game_objects = std::vector<GameObject *>();
    this->callback = [](std::vector<GameObject *> *) {};
}

bool GameEngine::Init() {
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

bool GameEngine::InitSingleClient() {
    bool display_success = this->InitializeDisplay();
    this->ShowWelcomeScreen();
    return display_success;
}

void GameEngine::CSServerClientThread(int player_id) {
    zmq::socket_t client_socket(this->zmq_context, zmq::socket_type::rep);
    client_socket.bind("tcp://*:600" + std::to_string(player_id));

    Log(LogLevel::Info, "Client thread for client [%d] started", player_id);

    while (!app->sigint.load()) {
        try {
            zmq::message_t request;
            zmq::recv_result_t result = client_socket.recv(request, zmq::recv_flags::none);
            ObjectUpdate object_update;
            std::memcpy(&object_update, request.data(), sizeof(ObjectUpdate));

            std::string ack = "Acknowledge client [" + std::to_string(player_id) + "]";
            zmq::message_t reply(ack.size());
            std::memcpy(reply.data(), ack.c_str(), ack.size());
            client_socket.send(reply, zmq::send_flags::none);

            GameObject *game_object = GetObjectByName(object_update.name, this->game_objects);
            game_object->SetPosition(object_update.position);
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the server client thread: %s", e.what());
            client_socket.close();
            this->server_broadcast_socket.close();
        }
    }
};

void GameEngine::CSServerBroadcastUpdates() {
    for (GameObject *game_object : this->game_objects) {
        try {
            ObjectUpdate object_update;
            std::snprintf(object_update.name, sizeof(object_update.name), "%s",
                          game_object->GetName().c_str());
            object_update.position = game_object->GetPosition();

            zmq::message_t broadcast_update(sizeof(ObjectUpdate));
            std::memcpy(broadcast_update.data(), &object_update, sizeof(ObjectUpdate));
            this->server_broadcast_socket.send(broadcast_update, zmq::send_flags::none);
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error while broadcasting server updates: %s", e.what());
            this->server_broadcast_socket.close();
        }
    }
}

void GameEngine::CSServerListenerThread() {
    Log(LogLevel::Info, "Server listening for incoming connections at port 5555");

    while (!app->sigint.load()) {
        try {
            zmq::message_t request;
            zmq::recv_result_t result = this->join_socket.recv(request, zmq::recv_flags::none);
            std::string message(static_cast<char *>(request.data()), request.size());

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

                std::thread client_thread(
                    [this, player_id]() { this->CSServerClientThread(player_id); });
                client_thread.detach();
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the server listener thread: %s", e.what());
            this->join_socket.close();
        }
    }
};

bool GameEngine::InitCSServer() {
    this->zmq_context = zmq::context_t(1);

    this->join_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::rep);
    this->join_socket.bind("tcp://*:5555");
    std::thread listener_thread([this]() { this->CSServerListenerThread(); });
    listener_thread.detach();

    this->server_broadcast_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::pub);
    this->server_broadcast_socket.bind("tcp://*:5556");

    return true;
}

bool GameEngine::InitCSClientConnection() {
    this->zmq_context = zmq::context_t(1);
    int join_port = 5555;
    int client_update_port = 6000;
    int server_broadcast_port = 5556;

    this->join_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::req);
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
    this->client_update_socket.connect(GetConnectionAddress(
        this->network_info.server_ip, client_update_port + this->network_info.id));

    this->server_broadcast_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::sub);
    this->server_broadcast_socket.connect(
        GetConnectionAddress(this->network_info.server_ip, server_broadcast_port));
    this->server_broadcast_socket.set(zmq::sockopt::subscribe, "");

    return true;
}

bool GameEngine::InitP2PPeerConnection() {
    this->zmq_context = zmq::context_t(1);
    int host_broadcast_port = 6001;
    int join_port = 5555;

    // The peer connects to the host peer's broadcast socket
    // One of the peers is called the 'host' since our design employs a listen-server architecture
    this->host_broadcast_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::sub);
    this->host_broadcast_socket.connect(
        GetConnectionAddress(this->network_info.host_ip, host_broadcast_port));
    this->host_broadcast_socket.set(zmq::sockopt::subscribe, "");

    // Connecting to the host in order to send a join message
    this->join_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::req);
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
    this->peer_broadcast_socket.bind("tcp://*:600" + std::to_string(this->network_info.id));

    return true;
}

bool GameEngine::InitCSClient() {
    bool display_success = this->InitializeDisplay();
    bool client_connection_success = this->InitCSClientConnection();
    this->ShowWelcomeScreen();

    return display_success && client_connection_success;
}

void GameEngine::P2PHostBroadcastPlayers() {
    for (GameObject *game_object : this->game_objects) {
        try {
            if (game_object->GetOwner() == Peer) {
                ObjectUpdate object_update;
                std::snprintf(object_update.name, sizeof(object_update.name), "%s",
                              game_object->GetName().c_str());
                object_update.position = game_object->GetPosition();
                std::snprintf(object_update.player_address, sizeof(object_update.player_address),
                              "%s", game_object->GetPlayerAddress().c_str());

                zmq::message_t broadcast_update(sizeof(ObjectUpdate));
                std::memcpy(broadcast_update.data(), &object_update, sizeof(ObjectUpdate));
                this->host_broadcast_socket.send(broadcast_update, zmq::send_flags::none);
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error while broadcasting host players: %s", e.what());
            this->host_broadcast_socket.close();
        }
    }
}

void GameEngine::P2PHostListenerThread() {
    Log(LogLevel::Info, "Host listening for incoming connections at port 5555");

    while (!app->quit.load() && !app->sigint.load()) {
        try {
            zmq::message_t request;
            zmq::recv_result_t result = this->join_socket.recv(request, zmq::recv_flags::none);
            std::string message(static_cast<char *>(request.data()), request.size());

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

                std::thread peer_thread([this, player_id, player_address]() {
                    this->P2PReceiveBroadcastThread(player_id, player_address);
                });
                peer_thread.detach();
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
};

bool GameEngine::InitP2PHost() {
    bool display_success = this->InitializeDisplay();

    this->zmq_context = zmq::context_t(1);

    this->join_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::rep);
    this->join_socket.bind("tcp://*:5555");
    std::thread listener_thread([this]() { this->P2PHostListenerThread(); });
    listener_thread.detach();

    this->host_broadcast_socket = zmq::socket_t(this->zmq_context, zmq::socket_type::pub);
    this->network_info.id = 1;
    this->players_connected.store(1);
    this->host_broadcast_socket.bind("tcp://*:600" +
                                     std::to_string(this->players_connected.load()));

    this->ShowWelcomeScreen();

    return display_success;
}

bool GameEngine::InitP2PPeer() {
    bool display_success = this->InitializeDisplay();
    bool connection_success = this->InitP2PPeerConnection();
    this->ShowWelcomeScreen();

    return display_success && connection_success;
}

void GameEngine::Start() {
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

void GameEngine::StartSingleClient() {
    this->SetupDefaultInputs();

    std::thread input_thread = std::thread([this]() { this->ReadInputsThread(); });

    this->engine_timeline.SetFrameTime(FrameTime{0, this->engine_timeline.GetTime(), 0});

    // Engine loop
    while (!app->quit.load() && !app->sigint.load()) {
        app->quit.store(this->HandleQuitEvent());
        this->GetTimeDelta();
        this->ApplyObjectPhysicsAndUpdates();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
        this->RenderScene();
    }

    if (input_thread.joinable()) {
        input_thread.join();
    }

    this->Shutdown();
}

void GameEngine::StartCSServer() {
    this->engine_timeline.SetFrameTime(FrameTime{0, this->engine_timeline.GetTime(), 0});

    while (!app->sigint.load()) {
        this->GetTimeDelta();
        this->ApplyObjectPhysicsAndUpdates();
        this->CSServerBroadcastUpdates();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
    }
}

void GameEngine::CSClientAddExistingPlayers() {
    for (int player_id = 1; player_id <= this->network_info.id; player_id++) {
        this->CreateNewPlayer(player_id);
    }
}

GameObject *GameEngine::CreateNewPlayer(int player_id, std::string player_address) {
    bool is_p2p = this->network_info.mode == NetworkMode::PeerToPeer;
    bool is_host = this->network_info.role == NetworkRole::Host;
    bool is_cs = this->network_info.mode == NetworkMode::ClientServer;
    bool is_server = this->network_info.role == NetworkRole::Server;
    bool is_client = this->network_info.role == NetworkRole::Client;

    GameObject *controllable = GetControllable(this->game_objects);
    std::string player_name = Split(controllable->GetName(), '_')[0];
    player_name += "_" + std::to_string(player_id);

    if (is_p2p && is_host) {
        if (player_id == 1) {
            controllable->SetOwner(NetworkRole::Host);
        }
    }
    if ((is_cs && is_server && player_id == 1) || (player_id == this->network_info.id)) {
        controllable->SetName(player_name);
        SetPlayerTexture(controllable, player_id, this->player_textures);
        if (this->show_player_border) {
            controllable->SetBorder(Border{true, Color{0, 0, 0, 255}});
        }
        return controllable;
    }
    if ((is_cs && is_server && player_id > 1) || (player_id != this->network_info.id)) {
        GameObject *player = new GameObject(player_name, controllable->GetCategory());
        player->SetColor(controllable->GetColor());
        player->SetSize(controllable->GetSize());
        player->SetTextureTemplate(controllable->GetTextureTemplate());
        player->SetCallback(controllable->GetCallback());
        player->SetOwner(controllable->GetOwner());
        if (is_p2p) {
            player->SetOwner(NetworkRole::Peer);
            if (!player_address.empty()) {
                player->SetPlayerAddress(player_address);
            }
        }
        SetPlayerTexture(player, player_id, this->player_textures);

        this->game_objects.push_back(player);
        return player;
    }

    return nullptr;
}

void GameEngine::CSClientReceiveBroadcastThread() {
    while (!app->quit.load() && !app->sigint.load()) {
        try {
            zmq::message_t message;
            zmq::recv_result_t res =
                this->server_broadcast_socket.recv(message, zmq::recv_flags::none);

            if (res) {
                ObjectUpdate object_update;
                std::memcpy(&object_update, message.data(), sizeof(ObjectUpdate));
                GameObject *game_object = GetObjectByName(object_update.name, this->game_objects);
                // If the object received does not already exist in the client, create it. Occurs
                // whenever a new client joins the game
                if (game_object == nullptr) {
                    int player_id = GetPlayerIdFromName(object_update.name);
                    game_object = this->CreateNewPlayer(player_id);
                }
                GameObject *player = GetClientPlayer(this->network_info.id, this->game_objects);

                if (game_object->GetName() != player->GetName()) {
                    game_object->SetPosition(object_update.position);
                }
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the client receive broadcast thread: %s",
                e.what());
            this->server_broadcast_socket.close();
        }
    }
}

void GameEngine::CSClientSendUpdate() {
    try {
        GameObject *player = GetClientPlayer(this->network_info.id, this->game_objects);
        ObjectUpdate object_update;
        std::snprintf(object_update.name, sizeof(object_update.name), "%s",
                      player->GetName().c_str());
        object_update.position = player->GetPosition();

        zmq::message_t update(sizeof(ObjectUpdate));
        std::memcpy(update.data(), &object_update, sizeof(ObjectUpdate));
        this->client_update_socket.send(update, zmq::send_flags::none);

        zmq::message_t server_ack;
        zmq::recv_result_t res = this->client_update_socket.recv(server_ack, zmq::recv_flags::none);

        std::string server_ack_message(static_cast<const char *>(server_ack.data()),
                                       server_ack.size());
    } catch (const zmq::error_t &e) {
        Log(LogLevel::Info, "Caught error in the client send update thread: %s", e.what());
        this->client_update_socket.close();
    }
}

void GameEngine::StartCSClient() {
    this->CSClientAddExistingPlayers();

    this->SetupDefaultInputs();

    std::thread input_thread = std::thread([this]() { this->ReadInputsThread(); });

    std::thread receive_broadcast_thread =
        std::thread([this]() { this->CSClientReceiveBroadcastThread(); });

    this->engine_timeline.SetFrameTime(FrameTime{0, this->engine_timeline.GetTime(), 0});

    // Engine loop
    while (!app->quit.load() && !app->sigint.load()) {
        app->quit.store(this->HandleQuitEvent());
        this->GetTimeDelta();
        this->ApplyObjectPhysicsAndUpdates();
        this->CSClientSendUpdate();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
        this->RenderScene();
    }

    if (input_thread.joinable()) {
        input_thread.join();
    }

    if (receive_broadcast_thread.joinable()) {
        receive_broadcast_thread.join();
    }

    this->Shutdown();
}

void GameEngine::P2PBroadcastUpdates() {
    for (GameObject *game_object : GetObjectsByRole(this->network_info, this->game_objects)) {
        try {
            ObjectUpdate object_update;
            std::snprintf(object_update.name, sizeof(object_update.name), "%s",
                          game_object->GetName().c_str());
            object_update.position = game_object->GetPosition();

            zmq::message_t broadcast_update(sizeof(ObjectUpdate));
            std::memcpy(broadcast_update.data(), &object_update, sizeof(ObjectUpdate));

            if (this->network_info.role == Host) {
                // The host peer broadcasts the positions of all host governed objects
                this->host_broadcast_socket.send(broadcast_update, zmq::send_flags::none);

            } else if (this->network_info.role == Peer &&
                       this->network_info.id == GetPlayerIdFromName(game_object->GetName())) {
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

void GameEngine::P2PReceiveBroadcastThread(int player_id, std::string player_address) {
    Log(LogLevel::Info, "Receiving broadcasts from player: [%d]", player_id);
    int peer_receive_port = 6000;

    zmq::socket_t peer_receive_socket(this->zmq_context, zmq::socket_type::sub);
    peer_receive_socket.connect(
        GetConnectionAddress(player_address, peer_receive_port + player_id));
    peer_receive_socket.set(zmq::sockopt::subscribe, "");

    while (!app->quit.load() && !app->sigint.load()) {
        try {
            zmq::message_t message;
            zmq::recv_result_t res = peer_receive_socket.recv(message, zmq::recv_flags::none);

            if (res) {
                ObjectUpdate object_update;
                std::memcpy(&object_update, message.data(), sizeof(ObjectUpdate));
                GameObject *game_object = GetObjectByName(object_update.name, this->game_objects);
                game_object->SetPosition(object_update.position);
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the peer receive broadcast thread: %s", e.what());
            peer_receive_socket.close();
        }
    }
}

void GameEngine::P2PReceiveBroadcastFromHostThread() {
    Log(LogLevel::Info, "Receiving broadcasts from the host");

    std::string discover_message = "discover";
    zmq::message_t discover_request(discover_message.size());
    std::memcpy(discover_request.data(), discover_message.c_str(), discover_message.size());
    this->join_socket.send(discover_request, zmq::send_flags::none);

    zmq::message_t server_reply;
    zmq::recv_result_t res = this->join_socket.recv(server_reply, zmq::recv_flags::none);

    while (!app->quit.load() && !app->sigint.load()) {
        try {
            zmq::message_t message;
            zmq::recv_result_t res =
                this->host_broadcast_socket.recv(message, zmq::recv_flags::none);

            if (res) {
                ObjectUpdate object_update;
                std::memcpy(&object_update, message.data(), sizeof(ObjectUpdate));
                GameObject *game_object = GetObjectByName(object_update.name, this->game_objects);
                // If the object received does not already exist in the client, create it. Occurs
                // whenever a new client joins the game
                if (game_object == nullptr) {
                    int player_id = GetPlayerIdFromName(object_update.name);
                    std::string player_address = object_update.player_address;
                    game_object = this->CreateNewPlayer(player_id);
                    // spawn a new thread to receive broadcasts from the new peer
                    if (player_id != 1) {
                        std::thread peer_thread([this, player_id, player_address]() {
                            this->P2PReceiveBroadcastThread(player_id, player_address);
                        });
                        peer_thread.detach();
                    }
                }
                GameObject *player = GetClientPlayer(this->network_info.id, this->game_objects);

                if (game_object->GetName() != player->GetName()) {
                    game_object->SetPosition(object_update.position);
                }
            }
        } catch (const zmq::error_t &e) {
            Log(LogLevel::Info, "Caught error in the peer receive broadcast from host thread: %s",
                e.what());
            this->host_broadcast_socket.close();
        }
    }
}

void GameEngine::StartP2P() {
    this->SetupDefaultInputs();
    std::thread receive_broadcast_thread;

    std::thread input_thread = std::thread([this]() { this->ReadInputsThread(); });

    this->CreateNewPlayer(this->network_info.id);

    if (this->network_info.role == NetworkRole::Peer) {
        receive_broadcast_thread =
            std::thread([this]() { this->P2PReceiveBroadcastFromHostThread(); });
    }

    this->engine_timeline.SetFrameTime(FrameTime{0, this->engine_timeline.GetTime(), 0});

    // Engine loop
    while (!app->quit.load() && !app->sigint.load()) {
        app->quit.store(this->HandleQuitEvent());
        this->GetTimeDelta();
        this->ApplyObjectPhysicsAndUpdates();
        this->P2PBroadcastUpdates();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
        this->RenderScene();
    }

    if (input_thread.joinable()) {
        input_thread.join();
    }

    if (this->network_info.role == NetworkRole::Peer) {
        if (receive_broadcast_thread.joinable()) {
            receive_broadcast_thread.join();
        }
    }

    this->Shutdown();
}

void GameEngine::SetupDefaultInputs() {
    // toggle constant and proportional scaling
    app->key_map->key_X.OnPress = [this]() {
        app->window.proportional_scaling = !app->window.proportional_scaling;
    };
    // toggle pause or unpause
    app->key_map->key_P.OnPress = [this]() {
        this->engine_timeline.TogglePause(this->engine_timeline.GetFrameTime().current);
    };
    // slow down the timeline
    app->key_map->key_comma.OnPress = [this]() {
        this->engine_timeline.ChangeTic(std::min(this->engine_timeline.GetTic() * 2.0, 2.0));
    };
    // speed up the timeline
    app->key_map->key_period.OnPress = [this]() {
        this->engine_timeline.ChangeTic(std::max(this->engine_timeline.GetTic() / 2.0, 0.5));
    };
}

bool GameEngine::InitializeDisplay() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        Log(LogLevel::Error, "SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    SDL_Window *window = SDL_CreateWindow(
        this->game_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        app->window.width, app->window.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

void GameEngine::SetGameTitle(std::string game_title) {
    if (this->network_info.role == NetworkRole::Client ||
        this->network_info.role == NetworkRole::Host ||
        this->network_info.role == NetworkRole::Peer) {
        this->game_title = game_title;
    }
}

void GameEngine::SetNetworkInfo(NetworkInfo network_info) { this->network_info = network_info; }

NetworkInfo GameEngine::GetNetworkInfo() { return this->network_info; }

void GameEngine::SetBackgroundColor(Color color) {
    if (this->network_info.role == NetworkRole::Client ||
        this->network_info.role == NetworkRole::Host ||
        this->network_info.role == NetworkRole::Peer) {
        this->background_color = color;
    }
}

void GameEngine::SetShowPlayerBorder(bool show_player_border) {
    this->show_player_border = show_player_border;
}

void GameEngine::SetPlayerTextures(int player_textures) { this->player_textures = player_textures; }

void GameEngine::SetMaxPlayers(int max_players) { this->max_players = max_players; }

void GameEngine::ShowWelcomeScreen() {
    // Sets the background to blue
    SDL_SetRenderDrawColor(app->renderer, this->background_color.red, this->background_color.green,
                           this->background_color.blue, 255);
    // Clears the renderer
    SDL_RenderClear(app->renderer);
    SDL_RenderPresent(app->renderer);
}

void GameEngine::AddObjects(std::vector<GameObject *> game_objects) {
    this->game_objects = game_objects;
}

void GameEngine::SetCallback(std::function<void(std::vector<GameObject *> *)> callback) {
    this->callback = callback;
}

void GameEngine::Update() { this->callback(&this->game_objects); }

void GameEngine::GetTimeDelta() {
    int64_t current = this->engine_timeline.GetTime();
    int64_t last = this->engine_timeline.GetFrameTime().last;
    int64_t delta = current - last;
    last = current;
    delta = std::clamp(delta, static_cast<int64_t>(0),
                       static_cast<int64_t>(16'000'000 / this->engine_timeline.GetTic()));

    this->engine_timeline.SetFrameTime(FrameTime{current, last, delta});
}

void GameEngine::ReadInputsThread() {
    while (!app->quit.load() && !app->sigint.load()) {
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

void GameEngine::ApplyObjectPhysicsAndUpdates() {
    std::vector<GameObject *> game_objects =
        GetObjectsByRole(this->network_info, this->game_objects);

    for (GameObject *game_object : game_objects) {
        game_object->Move(this->engine_timeline.GetFrameTime().delta);
        game_object->Update();
    }
}

void GameEngine::TestCollision() {
    for (int i = 0; i < this->game_objects.size() - 1; i++) {
        for (int j = i + 1; j < this->game_objects.size(); j++) {
            SDL_Rect object_1 = {
                static_cast<int>(std::round(this->game_objects[i]->GetPosition().x)),
                static_cast<int>(std::round(this->game_objects[i]->GetPosition().y)),
                this->game_objects[i]->GetSize().width, this->game_objects[i]->GetSize().height};
            SDL_Rect object_2 = {
                static_cast<int>(std::round(this->game_objects[j]->GetPosition().x)),
                static_cast<int>(std::round(this->game_objects[j]->GetPosition().y)),
                this->game_objects[j]->GetSize().width, this->game_objects[j]->GetSize().height};

            if (SDL_HasIntersection(&object_1, &object_2)) {
                this->game_objects[i]->AddCollider(this->game_objects[j]);
                this->game_objects[j]->AddCollider(this->game_objects[i]);
            } else {
                this->game_objects[i]->RemoveCollider(this->game_objects[j]);
                this->game_objects[j]->RemoveCollider(this->game_objects[i]);
            }
        }
    }
}

void GameEngine::HandleCollisions() {
    std::vector<std::thread> threads;

    auto handle_collision_thread = [](GameObject *game_object) {
        if (game_object->GetColliders().size() > 0) {
            for (GameObject *collider : game_object->GetColliders()) {
                int obj_x = static_cast<int>(std::round(game_object->GetPosition().x));
                int obj_y = static_cast<int>(std::round(game_object->GetPosition().y));

                int col_x = static_cast<int>(std::round(collider->GetPosition().x));
                int col_y = static_cast<int>(std::round(collider->GetPosition().y));

                int obj_width = game_object->GetSize().width;
                int obj_height = game_object->GetSize().height;
                int col_width = collider->GetSize().width;
                int col_height = collider->GetSize().height;

                int left_overlap = (obj_x + obj_width) - col_x;
                int right_overlap = (col_x + col_width) - obj_x;
                int top_overlap = (obj_y + obj_height) - col_y;
                int bottom_overlap = (col_y + col_height) - obj_y;

                int min_overlap = std::min(std::min(left_overlap, right_overlap),
                                           std::min(top_overlap, bottom_overlap));

                int pos_x = 0, pos_y = 0;
                if (min_overlap == left_overlap) {
                    pos_x = col_x - obj_width;
                    pos_y = obj_y;
                } else if (min_overlap == right_overlap) {
                    pos_x = col_x + col_width;
                    pos_y = obj_y;
                } else if (min_overlap == top_overlap) {
                    pos_x = obj_x;
                    pos_y = col_y - obj_height;
                } else if (min_overlap == bottom_overlap) {
                    pos_x = obj_x;
                    pos_y = col_y + col_height;
                }

                game_object->SetPosition(Position{float(pos_x), float(pos_y)});

                float vel_x = game_object->GetVelocity().x;
                float vel_y = game_object->GetVelocity().y;
                if (min_overlap == left_overlap || min_overlap == right_overlap) {
                    vel_x *= -game_object->GetRestitution();
                }
                if (min_overlap == top_overlap || min_overlap == bottom_overlap) {
                    vel_y *= -game_object->GetRestitution();
                }
                game_object->SetVelocity(Velocity{vel_x, vel_y});
            }
        }
    };

    // Iterate over each game object
    for (GameObject *game_object : GetObjectsByRole(this->network_info, this->game_objects)) {
        if (game_object->GetAffectedByCollision()) {
            // Spawn a new thread for each game object
            threads.emplace_back(handle_collision_thread, game_object);
        }
    }

    // Join all threads
    for (auto &thread : threads) {
        thread.join();
    }
}

bool GameEngine::HandleQuitEvent() {
    SDL_Event event;
    bool quit = false;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
    }
    return quit;
}

void GameEngine::RenderScene() {
    this->HandleScaling();

    this->RenderBackground();
    for (GameObject *game_object : this->game_objects) {
        game_object->Render();
    }
    SDL_RenderPresent(app->renderer);
}

void GameEngine::RenderBackground() {
    // Add conditions to change the background later
    // Add options to render an image as a background later
    SDL_SetRenderDrawColor(app->renderer, this->background_color.red, this->background_color.green,
                           this->background_color.blue, 255);
    SDL_RenderClear(app->renderer);
}

void GameEngine::HandleScaling() {
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

void GameEngine::Shutdown() {
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