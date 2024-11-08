#pragma once

#include "SDL_scancode.h"
#include <chrono>
#include <unordered_set>

class Chord {
  private:
    int chord_id;
    std::unordered_set<SDL_Scancode> keys;

    std::vector<std::pair<SDL_Scancode, bool>> buffer;
    std::chrono::steady_clock::time_point start;
    int64_t elapsed;
    int timeout;

    void Reset();
    void Trigger();
    bool BufferContainsChord();
    void FlushBuffer();
    void LogBuffer();

  public:
    Chord(int chord_id, std::unordered_set<SDL_Scancode> keys);

    void Process();
    void ProcessKeyPress(SDL_Scancode key, bool pressed);
    bool IsKeyInChord(SDL_Scancode key);
};