#pragma once

#include "Event.hpp"
#include "EventHandler.hpp"
#include "Types.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

class Replay : public EventHandler {
  public:
    static Replay &GetInstance() {
        static Replay instance;
        return instance;
    }

  private:
    Replay();

  public:
    Replay(Replay const &) = delete;
    void operator=(Replay const &) = delete;

  private:
    std::atomic<bool> is_recording;
    std::atomic<bool> is_replaying;

    SDL_Scancode record_key;
    SDL_Scancode replay_key;

    int64_t start_record_time;
    int64_t start_replay_time;

    std::shared_ptr<Entity> camera;

    std::mutex recorded_events_mutex;
    std::vector<Event> recorded_events;
    std::vector<std::pair<Entity *, std::pair<Position, double>>> start_record_transforms;
    std::vector<std::pair<Entity *, std::pair<Position, double>>> start_replay_transforms;

    void HandleReplayInput(Event &event);
    void SetStartTransforms(
        std::vector<std::pair<Entity *, std::pair<Position, double>>> &start_transforms);
    void ApplyStartTransforms(
        std::vector<std::pair<Entity *, std::pair<Position, double>>> &start_transforms);
    void AdjustRecordedEventTimes();
    void RaiseRecordedEvents();

    void StartRecord();
    void StopRecord();
    void StartReplay();
    void StopReplay();

    void ClearRecordedEvents();
    std::vector<Event> GetRecordedEvents();

  public:
    bool GetIsRecording();
    bool GetIsReplaying();
    void SetIsRecording(bool is_recording);
    void SetIsReplaying(bool is_replaying);

    void SetCamera(std::shared_ptr<Entity> camera);

    void BindRecordKey(SDL_Scancode key);
    void BindReplayKey(SDL_Scancode key);

    void RecordEvent(Event event);

    void OnEvent(Event event) override;
};