#include "Replay.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "EventManager.hpp"
#include "Transform.hpp"
#include "Types.hpp"
#include <vector>

Replay::Replay() {
    this->is_recording.store(false);
    this->is_replaying.store(false);

    this->record_key = SDL_SCANCODE_Q;
    this->replay_key = SDL_SCANCODE_R;

    this->recorded_events = std::vector<Event>();
    this->start_record_positions = std::vector<std::pair<Entity *, Position>>();
    this->start_replay_positions = std::vector<std::pair<Entity *, Position>>();

    EventManager::GetInstance().Register({EventType::Input, EventType::StartRecord,
                                          EventType::StopRecord, EventType::StartReplay,
                                          EventType::StopReplay},
                                         this);
}

bool Replay::GetIsRecording() { return this->is_recording.load(); }
bool Replay::GetIsReplaying() { return this->is_replaying.load(); }
void Replay::SetIsRecording(bool is_recording) { this->is_recording.store(is_recording); }
void Replay::SetIsReplaying(bool is_replaying) { this->is_replaying.store(is_replaying); }

void Replay::SetCamera(std::shared_ptr<Entity> camera) { this->camera = camera; }

void Replay::BindRecordKey(SDL_Scancode key) { this->record_key = key; }
void Replay::BindReplayKey(SDL_Scancode key) { this->replay_key = key; }

void Replay::RecordEvent(Event event) {
    std::lock_guard<std::mutex> lock(this->recorded_events_mutex);
    this->recorded_events.push_back(event);
}

void Replay::ClearRecordedEvents() {
    std::lock_guard<std::mutex> lock(this->recorded_events_mutex);
    this->recorded_events.clear();
}

std::vector<Event> Replay::GetRecordedEvents() {
    std::lock_guard<std::mutex> lock(this->recorded_events_mutex);
    return this->recorded_events;
}

void Replay::SetStartPositions(std::vector<std::pair<Entity *, Position>> &start_positions) {
    start_positions.clear();
    for (const auto &entity : Engine::GetInstance().GetEntities()) {
        start_positions.push_back({entity, entity->GetComponent<Transform>()->GetPosition()});
    }
    start_positions.push_back(
        {this->camera.get(), this->camera->GetComponent<Transform>()->GetPosition()});
}

void Replay::ApplyStartPositions(std::vector<std::pair<Entity *, Position>> &start_positions) {
    for (const auto &entity_position : start_positions) {
        Entity *entity = entity_position.first;
        Position position = entity_position.second;
        EventManager::GetInstance().RaiseMoveEvent(MoveEvent{entity, position});
    }
    start_positions.clear();
}

// Postpone the timestamps of the recorded events to a point immediately after replay start time
void Replay::AdjustRecordedEventTimes() {
    std::lock_guard<std::mutex> lock(this->recorded_events_mutex);

    for (Event &event : this->recorded_events) {
        event.SetTimestamp(this->start_replay_time +
                           (event.GetTimestamp() - this->start_record_time));
    }
}

void Replay::RaiseRecordedEvents() {
    for (Event event : this->GetRecordedEvents()) {
        EventManager::GetInstance().Raise(event);
    }
}

void Replay::StartRecord() {
    this->start_record_time = Engine::GetInstance().EngineTimelineGetFrameTime().current;
    this->is_recording.store(true);

    this->ClearRecordedEvents();
    this->SetStartPositions(this->start_record_positions);
}

void Replay::StopRecord() {
    this->is_recording.store(false);

    Event stop_replay = Event(EventType::StopReplay, StopReplayEvent{});
    stop_replay.SetDelay(-1);
    stop_replay.SetPriority(Priority::High);

    this->RecordEvent(stop_replay);
}

void Replay::StartReplay() {
    if (this->GetRecordedEvents().empty()) {
        return;
    }

    this->start_replay_time = EventManager::GetInstance().GetLastEventTimestamp();
    this->is_replaying.store(true);

    this->SetStartPositions(this->start_replay_positions);
    this->ApplyStartPositions(this->start_record_positions);
    this->AdjustRecordedEventTimes();
    this->RaiseRecordedEvents();
    this->ClearRecordedEvents();
}

void Replay::StopReplay() {
    this->ApplyStartPositions(this->start_replay_positions);

    this->is_replaying.store(false);
}

void Replay::HandleReplayInput(Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));
    if (input_event == nullptr) {
        return;
    }
    bool pressed = input_event->pressed;
    if (!pressed) {
        return;
    }

    SDL_Scancode key = input_event->key;
    if (key == this->record_key) {
        if (this->is_replaying.load()) {
            return;
        }

        if (!this->is_recording.load()) {
            EventManager::GetInstance().RaiseStartRecordEvent(StartRecordEvent{});
        } else {
            EventManager::GetInstance().RaiseStopRecordEvent(StopRecordEvent{});
        }
    }

    if (key == this->replay_key) {
        if (this->is_recording.load()) {
            return;
        }

        if (!this->is_replaying.load()) {
            EventManager::GetInstance().RaiseStartReplayEvent(StartReplayEvent{});
        }
    }
}

void Replay::OnEvent(Event event) {
    EventType event_type = event.type;

    switch (event_type) {
    case EventType::Input: {
        this->HandleReplayInput(event);
        break;
    }
    case EventType::StartRecord: {
        this->StartRecord();
        break;
    }
    case EventType::StopRecord: {
        this->StopRecord();
        break;
    }
    case EventType::StartReplay: {
        this->StartReplay();
        break;
    }
    case EventType::StopReplay: {
        this->StopReplay();
        break;
    }
    default:
        break;
    }
}