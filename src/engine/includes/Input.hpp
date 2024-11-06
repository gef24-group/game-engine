#pragma once

#include "SDL_scancode.h"
#include "SDL_stdinc.h"
#include <unordered_map>
#include <unordered_set>

class Input {
  private:
    Uint8 *keyboard_state;
    std::unordered_map<int, std::unordered_set<SDL_Scancode>> chords;
    std::unordered_map<SDL_Scancode, int64_t> key_press_times;
    int64_t chord_delay;

    void RaiseEvent(SDL_Scancode key, bool is_pressed);
    bool IsKeyInChords(SDL_Scancode key);

  public:
    Input();
    void Process();
    void RegisterInputChord(int chord_id, std::unordered_set<SDL_Scancode> chord);
};