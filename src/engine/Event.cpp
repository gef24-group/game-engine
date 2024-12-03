#include "Event.hpp"
#include "Engine.hpp"
#include "Types.hpp"

Event::Event(EventType type, EventData data) {
    this->type = type;
    this->data = data;
    this->delay = 0;
    this->timestamp = Engine::GetInstance().EngineTimelineGetFrameTime().current;
    this->priority = Priority::High;
}

int64_t Event::GetDelay() { return this->delay; }

// Gives the created event a delay of 'delay' milliseconds
void Event::SetDelay(int64_t delay) {
    this->delay = delay;

    if (delay == -1) {
        delay = 0;
    }
    this->timestamp =
        Engine::GetInstance().EngineTimelineGetFrameTime().current + (1'000'000 * delay);
}

void Event::SetPriority(Priority priority) { this->priority = priority; }

int64_t Event::GetTimestamp() const { return this->timestamp; }

void Event::SetTimestamp(int64_t timestamp) { this->timestamp = timestamp; }