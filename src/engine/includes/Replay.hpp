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
    std::vector<std::pair<Entity *, Position>> start_record_positions;
    std::vector<std::pair<Entity *, Position>> start_replay_positions;

    void HandleReplayInput(Event &event);
    void SetStartPositions(std::vector<std::pair<Entity *, Position>> &positions);
    void ApplyStartPositions(std::vector<std::pair<Entity *, Position>> &positions);
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