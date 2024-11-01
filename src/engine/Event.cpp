#include "Event.hpp"
#include "Engine.hpp"
#include "Types.hpp"

Event::Event(EventType type, EventData data) {
    this->type = type;
    this->data = data;
    this->timestamp = Engine::GetInstance().EngineTimelineGetTime();
    this->priority = Priority::High;
}

// Gives the created event a delay of 'delay' milliseconds
void Event::SetDelay(int64_t delay) {
    this->timestamp = Engine::GetInstance().EngineTimelineGetTime() + (1'000'000 * delay);
}

void Event::SetPriority(Priority priority) { this->priority = priority; }