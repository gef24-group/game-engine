#pragma once

#include "EventHandler.hpp"
#include "SDL_scancode.h"

class EngineHandler : public EventHandler {
  private:
    SDL_Scancode pause_key;
    SDL_Scancode speed_down_key;
    SDL_Scancode speed_up_key;
    SDL_Scancode display_scaling_key;
    SDL_Scancode hidden_zone_key;

  public:
    EngineHandler();

    void BindPauseKey(SDL_Scancode key);
    void BindSpeedDownKey(SDL_Scancode key);
    void BindSpeedUpKey(SDL_Scancode key);
    void BindDisplayScalingKey(SDL_Scancode key);
    void BindHiddenZoneKey(SDL_Scancode key);

    void OnEvent(Event event) override;
};