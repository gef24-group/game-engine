#pragma once
#include "SDL.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include "defs.hpp"

class GameEngine {
    private:
        // TODO: Make a struct named "App" in which win and ren are member variables.
        SDL_Window *win;
        SDL_Renderer *ren;
    public:
        GameEngine();
        void start();
        bool init(const char *gameTitle);
        bool initialize_display(const char *gameTitle);
        void show_welcome_screen(int r, int g, int b);
        void update();
        void prepare_scene();
        void present_scene();
        void shutdown();
};