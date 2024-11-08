#pragma once

#include "Chord.hpp"
#include "SDL_scancode.h"
#include "SDL_stdinc.h"
#include <chrono>
#include <unordered_set>
#include <vector>

class Input {
  private:
    Uint8 *keyboard_state;
    std::vector<Chord> chords;

    std::vector<std::pair<SDL_Scancode, bool>> buffer;
    std::chrono::steady_clock::time_point start;
    int64_t elapsed;
    int timeout;

    bool IsKeyInChords(SDL_Scancode key);
    int BufferContainsChord();
    void LogBuffer();
    void FlushBuffer();
    void TriggerChord(int chord_id);
    void ResetChords();

  public:
    Input();
    void Process();
    void RegisterInputChord(int chord_id, std::unordered_set<SDL_Scancode> keys);
};