#pragma once

#include <cstdint>

class Timeline {
  private:
    int64_t start_time;
    int64_t elapsed_paused_time;
    int64_t last_paused_time;
    double tic;
    bool paused;
    Timeline *anchor;

  public:
    Timeline(Timeline *anchor, double tic);
    Timeline();
    int64_t GetTime();
    int64_t GetLastPausedTime();
    void TogglePause(int64_t pause_time);
    void ChangeTic(double tic);
    double GetTic();
    bool IsPaused();
};