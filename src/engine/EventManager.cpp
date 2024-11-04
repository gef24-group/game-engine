#include "EventManager.hpp"
#include "Engine.hpp"
#include "EventHandler.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <unordered_set>
#include <vector>

#include "Profile.hpp"
PROFILED;

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
    if (event.GetDelay() == -1) {
        this->HandleEvent(event);
        return;
    }

    this->PushEventQueue(event);
}

void EventManager::HandleEvent(Event event) {
    auto iterator = this->handlers.find(event.type);
    if (iterator != this->handlers.end()) {
        std::unordered_set<EventHandler *> handlers = iterator->second;
        for (EventHandler *handler : handlers) {
            handler->OnEvent(event);
        }
    }
}

void EventManager::ProcessEvents() {
#ifdef PROFILE
    this->ProfileEventQueue();
#endif

    int64_t current_time = Engine::GetInstance().EngineTimelineGetFrameTime().current;

    while (!this->GetEventQueue().empty()) {
        Event event = this->GetEventQueue().top(); // Get the highest priority event

        if (event.timestamp > current_time) {
            break;
        }

        this->HandleEvent(event);
        this->PopEventQueue();
    }
}

void EventManager::ProfileEventQueue() {
    ZoneScoped;

    auto event_queue = this->GetEventQueue();

    std::string zone_text = "";
    while (!event_queue.empty()) {
        const auto &event = event_queue.top();

        switch (event.type) {
        case EventType::Input: {
            zone_text += "Input";
            const InputEvent *input_event = std::get_if<InputEvent>(&(event.data));
            if (input_event) {
                SDL_Scancode key = input_event->keys[0];
                zone_text += "_" + std::to_string(key);
            }

            break;
        }

        case EventType::Move: {
            zone_text += "Move";
            const MoveEvent *move_event = std::get_if<MoveEvent>(&(event.data));
            if (move_event) {
                zone_text += "_" + std::string(move_event->entity_name);
            }

            break;
        }

        case EventType::Collision: {
            zone_text += "Collision";
            const CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));
            if (collision_event) {
                zone_text += "_" + GetEntityByName((collision_event->collider_1->GetName()),
                                                   Engine::GetInstance().GetEntities())
                                       ->GetName();
                zone_text += "_" + GetEntityByName((collision_event->collider_2->GetName()),
                                                   Engine::GetInstance().GetEntities())
                                       ->GetName();
            }

            break;
        }

        case EventType::Spawn: {
            zone_text += "Spawn";
            const SpawnEvent *spawn_event = std::get_if<SpawnEvent>(&(event.data));
            if (spawn_event) {
                zone_text += "_" + GetEntityByName((spawn_event->entity->GetName()),
                                                   Engine::GetInstance().GetEntities())
                                       ->GetName();
            }

            break;
        }

        case EventType::Death: {
            zone_text += "Death";
            const DeathEvent *death_event = std::get_if<DeathEvent>(&(event.data));
            if (death_event) {
                zone_text += "_" + GetEntityByName((death_event->entity->GetName()),
                                                   Engine::GetInstance().GetEntities())
                                       ->GetName();
            }

            break;
        }

        default:
            break;
        }

        zone_text += "_" + std::to_string(event.timestamp);
        ZoneText(zone_text.c_str(), zone_text.size());

        zone_text = "";
        event_queue.pop();
    }
}

void EventManager::RaiseInputEvent(InputEvent event) {
    Event input_event = Event(EventType::Input, event);
    input_event.SetDelay(0);
    input_event.SetPriority(Priority::High);

    this->Raise(input_event);
}

void EventManager::RaiseCollisionEvent(CollisionEvent event) {
    Event collision_event = Event(EventType::Collision, event);
    collision_event.SetDelay(0);
    collision_event.SetPriority(Priority::Medium);

    this->Raise(collision_event);
}

void EventManager::RaiseDeathEvent(DeathEvent event) {
    auto event_queue = this->GetEventQueue();
    bool has_death_or_spawn_event = false;
    while (!event_queue.empty()) {
        const auto &event = event_queue.top();
        if (event.type == EventType::Death || event.type == EventType::Spawn) {
            has_death_or_spawn_event = true;
            break;
        }
        event_queue.pop();
    }
    if (has_death_or_spawn_event) {
        return;
    }

    Event death_event = Event(EventType::Death, event);
    death_event.SetDelay(0);
    death_event.SetPriority(Priority::Low);

    this->Raise(death_event);
}

void EventManager::RaiseSpawnEvent(SpawnEvent event) {
    Event spawn_event = Event(EventType::Spawn, event);
    spawn_event.SetDelay(1000);
    spawn_event.SetPriority(Priority::Low);

    this->Raise(spawn_event);
}

void EventManager::RaiseMoveEvent(MoveEvent event) {
    Event move_event = Event(EventType::Move, event);
    move_event.SetDelay(-1);
    move_event.SetPriority(Priority::High);

    this->Raise(move_event);
}

bool EventManager::HasDeathEvent() {
    auto event_queue = this->GetEventQueue();
    bool has_death_event = false;
    while (!event_queue.empty()) {
        const auto &event = event_queue.top();
        if (event.type == EventType::Death) {
            has_death_event = true;
            break;
        }
        event_queue.pop();
    }

    return has_death_event;
}

std::priority_queue<Event, std::vector<Event>, ComparePriority> EventManager::GetEventQueue() {
    std::lock_guard<std::mutex> lock(this->event_queue_mutex);
    return this->event_queue;
}

void EventManager::PushEventQueue(Event event) {
    std::lock_guard<std::mutex> lock(this->event_queue_mutex);
    this->event_queue.push(event);
}

void EventManager::PopEventQueue() {
    std::lock_guard<std::mutex> lock(this->event_queue_mutex);
    this->event_queue.pop();
}