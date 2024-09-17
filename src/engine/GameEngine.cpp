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
#include <string>
#include <thread>
#include <vector>
#include <zmq.hpp>

GameEngine::GameEngine() {
    app->window = nullptr;
    app->renderer = nullptr;
    this->game_title = "";
    this->engine_timeline = Timeline();
    this->background_color = Color{0, 0, 0, 255};
    this->game_objects = std::vector<GameObject *>();
    this->callback = [](std::vector<GameObject *> *) {};
    this->window = {1920, 1080, false};
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
        if (this->network_info.role == NetworkRole::Server) {
            return this->InitP2PServer();
        }
        if (this->network_info.role == NetworkRole::Peer) {
            return this->InitP2PPeer();
        }
    }

    return false;
}

bool GameEngine::InitSingleClient() {
    app->key_map = new KeyMap();
    bool display_success = InitializeDisplay();
    this->ShowWelcomeScreen();
    return display_success;
}

bool GameEngine::InitCSServer() {
    zmq::context_t context(1); // Initialize ZMQ context with one I/O thread

    // Function to handle client-specific communication
    auto client_handler = [this](zmq::context_t &context, int client_id,
                                 const std::string &client_address) {
        zmq::socket_t client_socket(context, zmq::socket_type::rep);
        client_socket.connect(client_address);

        Log(LogLevel::Info, "Client thread for ID %d started", client_id);

        while (true) {
            zmq::message_t request;
            // Receive message from the client
            client_socket.recv(request, zmq::recv_flags::none);
            std::string message(static_cast<char *>(request.data()), request.size());

            Log(LogLevel::Info, "Received from client %d : %s", client_id, message.c_str());

            // Send acknowledgment back
            std::string ack = "ACK from server to client " + std::to_string(client_id);
            zmq::message_t reply(ack.size());
            memcpy(reply.data(), ack.c_str(), ack.size());
            client_socket.send(reply, zmq::send_flags::none);
        }
    };

    // Server to listen for initial "hello" and "join" messages
    auto server_listener = [this, &client_handler](zmq::context_t &context) {
        zmq::socket_t server_socket(context, zmq::socket_type::rep);
        server_socket.bind("tcp://*:5555");

        Log(LogLevel::Info, "Server listening for incoming connections...");

        while (true) {
            zmq::message_t request;
            // Receive message from the client
            server_socket.recv(request, zmq::recv_flags::none);
            std::string message(static_cast<char *>(request.data()), request.size());

            if (message == "join") {
                int client_id = this->clients_connected.fetch_add(1);
                std::string client_address = "tcp://localhost:600" + std::to_string(client_id);

                Log(LogLevel::Info, "Received join message, assigning ID %d", client_id);

                // Reply to the client with its assigned ID
                std::string reply =
                    "ID: " + std::to_string(client_id) + " | Address: " + client_address;
                zmq::message_t reply_msg(reply.size());
                memcpy(reply_msg.data(), reply.c_str(), reply.size());
                server_socket.send(reply_msg, zmq::send_flags::none);

                // Create a new thread for client-specific communication
                std::thread client_thread(client_handler, std::ref(context), client_id,
                                          client_address);
                client_thread.detach(); // Detach the thread to allow infinite running
            }
        }
    };

    // Start the server listener in a separate thread
    std::thread server_thread(server_listener, std::ref(context));
    server_thread.join(); // Keep the main thread alive
    return false;
}

// Initialize the client's connection to the server
bool GameEngine::InitCSClientConnection() {
    // Sending a connection request to the server
    zmq::context_t context(1);
    zmq::socket_t req_socket(context, zmq::socket_type::req);
    req_socket.connect("tcp://localhost:5555");

    std::string join_message = "join";
    zmq::message_t connection_request(join_message.size());
    memcpy(connection_request.data(), join_message.c_str(), join_message.size());
    req_socket.send(connection_request, zmq::send_flags::none);

    zmq::message_t server_reply;
    auto res = req_socket.recv(server_reply, zmq::recv_flags::none);

    std::string client_id(static_cast<const char *>(server_reply.data()), server_reply.size());
    Log(LogLevel::Info, "The client ID assigned by the server: %s", client_id.c_str());
    this->network_info.id = std::stoi(client_id);

    return true;
}

bool GameEngine::InitCSClient() {
    app->key_map = new KeyMap();
    bool display_success = InitializeDisplay();
    bool client_connection_success = InitCSClientConnection();
    this->ShowWelcomeScreen();

    return display_success && client_connection_success;
}

bool GameEngine::InitP2PServer() { return false; }

bool GameEngine::InitP2PPeer() { return false; }

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
        if (this->network_info.role == NetworkRole::Server) {
            this->StartP2PServer();
        }
        if (this->network_info.role == NetworkRole::Peer) {
            this->StartP2PPeer();
        }
    }
}

void GameEngine::StartSingleClient() {
    app->quit = false;

    this->SetupDefaultInputs();

    std::thread input_thread = std::thread([this]() {
        while (!app->quit) {
            this->ReadHIDs();
        }
    });

    this->engine_timeline.SetFrameTime(FrameTime{0, this->engine_timeline.GetTime(), 0});

    // Engine loop
    while (!app->quit) {
        app->quit = this->HandleEvents();
        this->GetTimeDelta();
        this->ApplyObjectPhysics();
        this->ApplyObjectUpdates();
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

void GameEngine::StartCSServer() {}

void GameEngine::ListenServerBroadcasts(zmq::context_t &context) {
    zmq::socket_t sub_socket(context, ZMQ_SUB);
    sub_socket.connect("tcp://localhost:600" + std::to_string(this->network_info.id));
    sub_socket.set(zmq::sockopt::subscribe, ""); // Subscribe to all messages

    while (!app->quit) {
        zmq::message_t message;
        auto res = sub_socket.recv(message, zmq::recv_flags::none);
        std::string received_message(static_cast<char *>(message.data()), message.size());
        // std::cout << "Broadcast message: " << received_message << std::endl;
        Log(LogLevel::Info, "Broadcast message received from the server: %s",
            received_message.c_str());
    }
}

void GameEngine::StartCSClient() {
    app->quit = false;

    zmq::context_t context(1);
    std::thread listener_thread =
        std::thread([this, &context]() { this->ListenServerBroadcasts(context); });

    // std::thread input_thread = std::thread([this]() {
    //     while (!app->quit) {
    //         this->ReadHIDs();
    //     }
    // });

    // this->engine_timeline.SetFrameTime(FrameTime{0, this->engine_timeline.GetTime(), 0});
    // // Engine loop
    // while (!app->quit) {
    //     app->quit = this->HandleEvents();
    //     this->GetTimeDelta();
    //     this->ApplyObjectPhysics();
    //     this->ApplyObjectUpdates();
    //     this->TestCollision();
    //     this->HandleCollisions();
    //     this->Update();
    //     this->RenderScene();
    // }

    // if (input_thread.joinable() && listener_thread.joinable()) {
    //     input_thread.join();
    //     listener_thread.join();
    // }
    if (listener_thread.joinable()) {
        listener_thread.join();
    }

    this->Shutdown();
}
void GameEngine::StartP2PServer() {}

void GameEngine::StartP2PPeer() {}

void GameEngine::SetupDefaultInputs() {
    // toggle constant and proportional scaling
    app->key_map->key_X.OnPress = [this]() {
        this->window.proportional_scaling = !this->window.proportional_scaling;
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
        this->window.width, this->window.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        Log(LogLevel::Error, "SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return false;
    } else {
        app->window = window;
    }

    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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

void GameEngine::SetGameTitle(std::string game_title) { this->game_title = game_title; }

void GameEngine::SetNetworkInfo(NetworkInfo network_info) { this->network_info = network_info; }

NetworkInfo GameEngine::GetNetworkInfo() { return this->network_info; }

void GameEngine::SetBackgroundColor(Color color) { this->background_color = color; }

void GameEngine::ShowWelcomeScreen() {
    // Sets the background to blue
    SDL_SetRenderDrawColor(app->renderer, this->background_color.red, this->background_color.green,
                           this->background_color.blue, 255);
    // Clears the renderer
    SDL_RenderClear(app->renderer);
    SDL_RenderPresent(app->renderer);
}

std::vector<GameObject *> GameEngine::GetObjects() { return this->game_objects; }

void GameEngine::AddObjects(std::vector<GameObject *> game_objects) {
    // TODO: Insert game objects in such a way that "Controllable" ones comes first, followed by
    // "Moveable" and then "Stationary"
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

void GameEngine::ReadHIDs() {
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    auto now = std::chrono::high_resolution_clock::now();

    auto debounce_key = [&](int scancode, Key &key, bool delay) {
        if (!delay) {
            key.pressed.store(keyboard_state[scancode] != 0);
            return;
        }

        if (keyboard_state[scancode] != 0) {
            auto press_duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(now - key.last_pressed_time);
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

void GameEngine::ApplyObjectPhysics() {
    for (GameObject *game_object : this->game_objects) {
        game_object->Move(this->engine_timeline.GetFrameTime().delta);
    }
}

void GameEngine::ApplyObjectUpdates() {
    for (GameObject *game_object : this->game_objects) {
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

    auto handle_collision = [&](GameObject *game_object) {
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
    for (GameObject *game_object : this->game_objects) {
        if (game_object->GetAffectedByCollision()) {
            // Spawn a new thread for each game object
            threads.emplace_back(handle_collision, game_object);
        }
    }

    // Join all threads
    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

bool GameEngine::HandleEvents() {
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

    if (this->window.proportional_scaling) {
        set_logical_size_err =
            SDL_RenderSetLogicalSize(app->renderer, this->window.width, this->window.height);
    } else {
        set_logical_size_err = SDL_RenderSetLogicalSize(app->renderer, 0, 0);
    }

    if (set_logical_size_err) {
        Log(LogLevel::Error, "Set Viewport failed: %s", SDL_GetError());
    }
}

void GameEngine::Shutdown() {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
    delete app->key_map;
    delete app;
}