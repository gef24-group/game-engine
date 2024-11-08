#pragma once

#include "Chord.hpp"
#include "SDL_scancode.h"
#include "SDL_stdinc.h"
#include <unordered_set>

class Input {
  private:
    Uint8 *keyboard_state;
    std::vector<Chord> chords;

    bool IsKeyInChords(SDL_Scancode key);

  public:
    Input();
    void Process();
    void RegisterInputChord(int chord_id, std::unordered_set<SDL_Scancode> keys);
};