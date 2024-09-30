#pragma once

#include "Types.hpp"
#include <cstdint>
#include <mutex>

class Timeline {
  private:
    std::mutex timeline_mutex;
    int64_t start_time;
    int64_t elapsed_paused_time;
    int64_t last_paused_time;
    FrameTime frame_time;
    double tic;
    bool paused;
    Timeline *anchor;

  public:
    Timeline(Timeline *anchor, double tic);
    Timeline();
    int64_t GetTime();
    FrameTime GetFrameTime();
    void SetFrameTime(FrameTime frame_time);
    void TogglePause(int64_t pause_time);
    void ChangeTic(double tic);
    double GetTic();
    bool IsPaused();
};