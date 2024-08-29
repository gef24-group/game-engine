#include "GameEngine.hpp"
#include "Constants.hpp"
#include "GameObject.hpp"
#include "SDL.h"
#include <iostream>
#include <stdio.h>

GameEngine::GameEngine() : win(nullptr), ren(nullptr) {}

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
    }

    // Code for graceful shutdown
    this->Shutdown();
}

bool GameEngine::Init(const char *game_title) {
    bool display_success = InitializeDisplay(game_title);
    this->ShowWelcomeScreen(WELCOME_SCREEN_RED, WELCOME_SCREEN_GREEN, WELCOME_SCREEN_BLUE);

    return display_success;
}

bool GameEngine::InitializeDisplay(const char *game_title) {
    // Referred https://www.willusher.io/sdl2%20tutorials/2013/08/17/lesson-1-hello-world/
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Window *win = SDL_CreateWindow(game_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                       SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!win) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    } else {
        this->win = win;
    }

    SDL_Renderer *ren =
        SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        SDL_DestroyWindow(win);
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    } else {
        this->ren = ren;
    }

    return true;
}

void GameEngine::ShowWelcomeScreen(int red, int green, int blue) {
    // Sets the background to blue
    SDL_SetRenderDrawColor(this->ren, red, green, blue, WELCOME_SCREEN_OPACITY);
    // Clears the renderer
    SDL_RenderClear(this->ren);
    SDL_RenderPresent(this->ren);
}

void GameEngine::AddObject(GameObject game_object) {
    // TODO: Insert game objects in such a way that "Controllable" ones comes first, followed by
    // "Moveable" and then "Stationary"
    this->gameObjects.push_back(game_object);
}

void GameEngine::Shutdown() {
    SDL_DestroyRenderer(this->ren);
    SDL_DestroyWindow(this->win);
}