#include "Timeline.hpp"
#include <chrono>

Timeline::Timeline() {
    auto now = std::chrono::high_resolution_clock::now();
    int64_t start_time = static_cast<int64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());

    this->anchor = nullptr;
    this->tic = 1;
    this->elapsed_paused_time = 0;
    this->last_paused_time = 0;
    this->paused = false;
    this->start_time = start_time;
}

Timeline::Timeline(Timeline *anchor, double tic) {
    this->anchor = anchor;
    this->tic = tic;
    this->elapsed_paused_time = 0;
    this->last_paused_time = 0;
    this->paused = false;
    this->start_time = this->anchor->GetTime();
}

int64_t Timeline::GetTime() {
    std::lock_guard<std::mutex> lock(this->timeline_mutex);

    auto now = std::chrono::high_resolution_clock::now();
    int64_t current_time;

    if (this->anchor == nullptr) {
        current_time = static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
    } else {
        current_time = this->anchor->GetTime();
    }

    if (this->paused) {
        return this->last_paused_time;
    }

    return static_cast<int64_t>(
               (static_cast<double>((current_time - this->start_time)) / this->tic)) -
           this->elapsed_paused_time;
}

FrameTime Timeline::GetFrameTime() {
    std::lock_guard<std::mutex> lock(this->timeline_mutex);
    return this->frame_time;
}

void Timeline::SetFrameTime(FrameTime frame_time) {
    std::lock_guard<std::mutex> lock(this->timeline_mutex);
    this->frame_time = frame_time;
}

void Timeline::TogglePause(int64_t pause_time) {
    int64_t time = this->GetTime();

    std::lock_guard<std::mutex> lock(this->timeline_mutex);

    this->paused = !this->paused;
    if (this->paused) {
        this->last_paused_time = pause_time;
    } else {
        this->elapsed_paused_time += time - this->last_paused_time;
    }
}

void Timeline::ChangeTic(double tic) {
    std::lock_guard<std::mutex> lock(this->timeline_mutex);
    this->tic = tic;
}

double Timeline::GetTic() {
    std::lock_guard<std::mutex> lock(this->timeline_mutex);
    return this->tic;
}

bool Timeline::IsPaused() {
    std::lock_guard<std::mutex> lock(this->timeline_mutex);
    return this->paused;
}