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
    this->timestamp =
        Engine::GetInstance().EngineTimelineGetFrameTime().current + (1'000'000 * this->delay);
}

void Event::SetPriority(Priority priority) { this->priority = priority; }