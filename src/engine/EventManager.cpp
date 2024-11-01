#include "EventManager.hpp"
#include "Engine.hpp"
#include "EventHandler.hpp"
#include "Types.hpp"
#include <unordered_set>
#include <vector>

void EventManager::Register(std::vector<EventType> event_types, EventHandler *handler) {
    for (EventType event_type : event_types) {
        this->handlers[event_type].insert(handler);
    }
}

void EventManager::Deregister(std::vector<EventType> event_types, EventHandler *handler) {
    for (EventType event_type : event_types) {
        auto iterator = std::find(this->handlers[event_type].begin(),
                                  this->handlers[event_type].end(), handler);

        if (iterator != this->handlers[event_type].end()) {
            this->handlers[event_type].erase(iterator);
        }
    }
}

void EventManager::Raise(Event event) {
    std::lock_guard<std::mutex> lock(this->event_queue_mutex);
    this->event_queue.push(event);
}

void EventManager::ProcessEvents() {
    std::lock_guard<std::mutex> lock(this->event_queue_mutex);

    int64_t current_time = Engine::GetInstance().EngineTimelineGetTime();

    while (!event_queue.empty()) {
        Event event = event_queue.top(); // Get the highest priority event

        if (event.timestamp > current_time) {
            break;
        }

        // Handle the event here
        auto iterator = this->handlers.find(event.type);
        if (iterator != this->handlers.end()) {
            std::unordered_set<EventHandler *> handlers = iterator->second;
            for (EventHandler *handler : handlers) {
                handler->OnEvent(event);
            }
        }

        event_queue.pop();
    }
}