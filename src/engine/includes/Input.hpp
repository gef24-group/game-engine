#pragma once

#include "SDL_scancode.h"
#include "SDL_stdinc.h"

class Input {
  private:
    Uint8 *keyboard_state;

    void RaiseEvent(SDL_Scancode key, bool is_pressed);

  public:
    Input();
    void Process();
};