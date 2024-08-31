#include "GameEngine.hpp"
#include "Constants.hpp"
#include "GameObject.hpp"
#include "SDL.h"
#include "SDL_keyboard.h"
#include "SDL_log.h"
#include "SDL_render.h"
#include <stdio.h>
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
        // Test collision
        // Stick colliders to boundary
        this->Update();
        this->RenderScene();
    }
}

bool GameEngine::Init(const char *game_title) {
    bool display_success = InitializeDisplay(game_title);
    this->ShowWelcomeScreen();

    return display_success;
}

bool GameEngine::InitializeDisplay(const char *game_title) {
    // Referred https://www.willusher.io/sdl2%20tutorials/2013/08/17/lesson-1-hello-world/
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "SDL_Init Error: %s",
                       SDL_GetError());
        return false;
    }

    SDL_Window *window =
        SDL_CreateWindow(game_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                         SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                       "SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return false;
    } else {
        app->window = window;
    }

    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                       "SDL_CreateRenderer Error: %s", SDL_GetError());
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

void GameEngine::ReadHIDs() { this->keyboard_state = SDL_GetKeyboardState(NULL); }

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

void GameEngine::RenderScene() {
    this->RenderBackground();
    for (GameObject *game_object : this->game_objects) {
        this->RenderObject(game_object);
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

void GameEngine::RenderObject(GameObject *game_object) {
    float pos_x = game_object->GetPosition().x;
    float pos_y = game_object->GetPosition().y;
    int width = game_object->GetSize().width;
    int height = game_object->GetSize().height;
    int red = game_object->GetColor().red;
    int green = game_object->GetColor().green;
    int blue = game_object->GetColor().blue;
    int alpha = game_object->GetColor().alpha;

    SDL_Rect object = {static_cast<int>(std::round(pos_x)), static_cast<int>(std::round(pos_y)),
                       width, height};
    SDL_SetRenderDrawColor(app->renderer, red, green, blue, alpha);
    if (game_object->GetShape() == Rectangle && game_object->GetTexture() == nullptr) {
        SDL_RenderFillRect(app->renderer, &object);
    } else {
        SDL_RenderCopy(app->renderer, game_object->GetTexture(), NULL, &object);
    }
}

void GameEngine::Shutdown() {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
}