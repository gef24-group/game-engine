#include "GameEngine.hpp"
#include "Constants.hpp"
#include "SDL.h"
#include <iostream>
#include <stdio.h>

GameEngine::GameEngine() {
    this->win = nullptr;
    this->ren = nullptr;
}

void GameEngine::start() {
    bool quit = false;

    // Game loop
    while (!quit) {
        // Referred https://www.willusher.io/sdl2%20tutorials/2013/08/17/lesson-1-hello-world/
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    // Code for graceful shutdown
    this->shutdown();
}

bool GameEngine::init(const char *gameTitle) {
    bool displaySuccess = initialize_display(gameTitle);
    this->show_welcome_screen(0, 0, 255);

    return displaySuccess;
}

bool GameEngine::initialize_display(const char *gameTitle) {
    // Referred https://www.willusher.io/sdl2%20tutorials/2013/08/17/lesson-1-hello-world/
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Window *win = SDL_CreateWindow(gameTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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

void GameEngine::show_welcome_screen(int r, int g, int b) {
    // Sets the background to blue
    SDL_SetRenderDrawColor(this->ren, r, g, b, 255);
    // Clears the renderer
    SDL_RenderClear(this->ren);
    SDL_RenderPresent(this->ren);
}

void GameEngine::shutdown() {
    SDL_DestroyRenderer(this->ren);
    SDL_DestroyWindow(this->win);
}