#pragma once

#include "SDL_render.h"
#include "SDL_video.h"

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