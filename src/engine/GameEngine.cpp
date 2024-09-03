#include "GameEngine.hpp"
#include "Constants.hpp"
#include "GameObject.hpp"
#include "SDL.h"
#include "SDL_keyboard.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include "Types.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <vector>

GameEngine::GameEngine() {
    app->window = nullptr;
    app->renderer = nullptr;
    this->background_color = Color{0, 0, 0, 255};
    this->game_objects = std::vector<GameObject *>();
    this->callback = [](std::vector<GameObject *>) {};
}

void GameEngine::Start() {
    bool quit = false;

    // Game loop
    while (!quit) {
        // Referred https://www.willusher.io/sdl2%20tutorials/2013/08/17/lesson-1-hello-world/
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        this->ReadHIDs();
        this->ApplyObjectPhysics(0.1);
        this->ApplyObjectUpdates();
        this->TestCollision();
        this->HandleCollisions();
        this->Update();
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
    // Referred https://www.willusher.io/sdl2%20tutorials/2013/08/17/lesson-1-hello-world/
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        Log(LogLevel::Error, "SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    SDL_Window *window =
        SDL_CreateWindow(game_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                         SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

void GameEngine::SetCallback(std::function<void(std::vector<GameObject *>)> callback) {
    this->callback = callback;
}

void GameEngine::Update() { this->callback(this->game_objects); }

void GameEngine::ReadHIDs() {
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    //  Iterating all the game objects
    app->key_map->key_W = keyboard_state[SDL_SCANCODE_W] != 0;
    app->key_map->key_A = keyboard_state[SDL_SCANCODE_A] != 0;
    app->key_map->key_S = keyboard_state[SDL_SCANCODE_S] != 0;
    app->key_map->key_D = keyboard_state[SDL_SCANCODE_D] != 0;
    app->key_map->key_up = keyboard_state[SDL_SCANCODE_UP] != 0;
    app->key_map->key_left = keyboard_state[SDL_SCANCODE_LEFT] != 0;
    app->key_map->key_down = keyboard_state[SDL_SCANCODE_DOWN] != 0;
    app->key_map->key_right = keyboard_state[SDL_SCANCODE_RIGHT] != 0;
}

void GameEngine::ApplyObjectPhysics(float time) {
    for (GameObject *game_object : this->game_objects) {
        game_object->Move(time);
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
    for (GameObject *game_object : this->game_objects) {
        if (game_object->GetCategory() == Controllable || game_object->GetCategory() == Moving) {
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

                    // find the overlap with respect to the collider
                    int left_overlap = (obj_x + obj_width) - col_x;
                    int right_overlap = (col_x + col_width) - obj_x;
                    int top_overlap = (obj_y + obj_height) - col_y;
                    int bottom_overlap = (col_y + col_height) - obj_y;

                    // find the side with the smallest overlap (this is where the collision
                    // occured)
                    int min_overlap = std::min(std::min(left_overlap, right_overlap),
                                               std::min(top_overlap, bottom_overlap));

                    // align object to the side with the lowest overlap
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

                    // update position to stick with collider and velocity to reduce with
                    // restitution
                    game_object->SetPosition(Position{float(pos_x), float(pos_y)});
                    if (game_object->GetCategory() == Controllable) {
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
            }
        }
    }
}

void GameEngine::RenderScene() {
    this->RenderBackground();
    for (GameObject *game_object : this->game_objects) {
        game_object->Render();
    }
    SDL_RenderPresent(app->renderer);
    SDL_Delay(SDL_DELAY);
}

void GameEngine::RenderBackground() {
    // Add conditions to change the background later
    // Add options to render an image as a background later
    SDL_SetRenderDrawColor(app->renderer, this->background_color.red, this->background_color.green,
                           this->background_color.blue, 255);
    SDL_RenderClear(app->renderer);
}

void GameEngine::Shutdown() {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
    delete app->key_map;
    delete app;
}