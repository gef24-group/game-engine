#pragma once

#include "SDL_scancode.h"
#include <unordered_set>

class Chord {
  private:
    int chord_id;
    std::unordered_set<SDL_Scancode> keys;

  public:
    Chord(int chord_id, std::unordered_set<SDL_Scancode> keys);

    int GetChordID();
    std::unordered_set<SDL_Scancode> GetKeys();
    bool IsKeyInChord(SDL_Scancode key);
};