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
#include <thread>
#include <vector>

GameEngine::GameEngine() {
    app->window = nullptr;
    app->renderer = nullptr;
    this->engine_timeline = Timeline();
    this->background_color = Color{0, 0, 0, 255};
    this->game_objects = std::vector<GameObject *>();
    this->callback = [](std::vector<GameObject *> *) {};
    this->window = {1920, 1080, false};
}

void GameEngine::Start() {
    app->quit = false;
    this->engine_timeline.SetFrameTime(FrameTime{0, this->engine_timeline.GetTime(), 0});

    // Engine loop
    while (!app->quit) {
        app->quit = this->HandleEvents();
        this->GetTimeDelta();
        this->ReadHIDs();
        this->ApplyObjectPhysics();
        this->ApplyObjectUpdates();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
        this->HandleTimelineInputs();
        this->RenderScene();
    }
    this->Shutdown();
}

bool GameEngine::Init(const char *game_title) {
    app->key_map = new KeyMap();
    bool display_success = InitializeDisplay(game_title);
    this->ShowWelcomeScreen();

    return display_success;
}

bool GameEngine::InitializeDisplay(const char *game_title) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        Log(LogLevel::Error, "SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    SDL_Window *window = SDL_CreateWindow(
        game_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, this->window.width,
        this->window.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

void GameEngine::SetNetworkInfo(NetworkInfo network_info) { this->network_info = network_info; }

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
            key.pressed = keyboard_state[scancode] != 0;
            return;
        }

        if (keyboard_state[scancode] != 0) {
            auto press_duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(now - key.last_pressed_time);
            if (press_duration.count() > 50 && !key.pressed) {
                key.pressed = true;
            } else {
                key.pressed = false;
                key.last_pressed_time = now;
            }
        } else {
            key.pressed = false;
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

                if (game_object->GetReduceVelocityOnCollision()) {
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
                // }
            }
        }
    };

    // Iterate over each game object
    for (GameObject *game_object : this->game_objects) {
        if (game_object->GetCategory() == Controllable || game_object->GetCategory() == Moving) {
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

void GameEngine::HandleTimelineInputs() {
    if (app->key_map->key_P.pressed) {
        this->engine_timeline.TogglePause(this->engine_timeline.GetFrameTime().current);
    }
    if (app->key_map->key_comma.pressed) {
        this->engine_timeline.ChangeTic(std::min(this->engine_timeline.GetTic() * 2.0, 2.0));
    }
    if (app->key_map->key_period.pressed) {
        this->engine_timeline.ChangeTic(std::max(this->engine_timeline.GetTic() / 2.0, 0.5));
    }
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

    if (app->key_map->key_X.pressed) {
        this->window.proportional_scaling = !this->window.proportional_scaling;
    }

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