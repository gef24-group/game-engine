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
    std::lock_guard<std::mutex> lock(this->handlers_mutex);

    for (EventType event_type : event_types) {
        this->handlers[event_type].insert(handler);
    }
}

void EventManager::Deregister(std::vector<EventType> event_types, EventHandler *handler) {
    std::lock_guard<std::mutex> lock(this->handlers_mutex);

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
    auto handlers = this->GetHandlers();

    auto iterator = handlers.find(event.type);
    if (iterator != handlers.end()) {
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
                SDL_Scancode key = input_event->key;
                zone_text += "_" + std::to_string(key);
            }

            break;
        }

        case EventType::Move: {
            zone_text += "Move";
            const MoveEvent *move_event = std::get_if<MoveEvent>(&(event.data));
            if (move_event) {
                zone_text += "_" + std::string(move_event->entity->GetName());
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
    if (this->IsDeathOrSpawnInQueue()) {
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

void EventManager::RaiseJoinEvent(JoinEvent event) {
    Event join_event = Event(EventType::Join, event);
    join_event.SetDelay(-1);
    join_event.SetPriority(Priority::High);

    this->Raise(join_event);
}

void EventManager::RaiseDiscoverEvent(DiscoverEvent event) {
    Event discover_event = Event(EventType::Discover, event);
    discover_event.SetDelay(-1);
    discover_event.SetPriority(Priority::High);

    this->Raise(discover_event);
}

void EventManager::RaiseLeaveEvent(LeaveEvent event) {
    Event leave_event = Event(EventType::Leave, event);
    leave_event.SetDelay(-1);
    leave_event.SetPriority(Priority::High);

    this->Raise(leave_event);
}

bool EventManager::IsDeathOrSpawnInQueue() {
    auto event_queue = this->GetEventQueue();
    bool is_death_or_spawn_in_queue = false;
    while (!event_queue.empty()) {
        const auto &event = event_queue.top();
        if (event.type == EventType::Death || event.type == EventType::Spawn) {
            is_death_or_spawn_in_queue = true;
            break;
        }
        event_queue.pop();
    }
    return is_death_or_spawn_in_queue;
}

std::priority_queue<Event, std::vector<Event>, ComparePriority> EventManager::GetEventQueue() {
    std::lock_guard<std::mutex> lock(this->event_queue_mutex);
    return this->event_queue;
}

std::unordered_map<EventType, std::unordered_set<EventHandler *>> EventManager::GetHandlers() {
    std::lock_guard<std::mutex> lock(this->handlers_mutex);
    return this->handlers;
}

void EventManager::PushEventQueue(Event event) {
    std::lock_guard<std::mutex> lock(this->event_queue_mutex);
    this->event_queue.push(event);
}

void EventManager::PopEventQueue() {
    std::lock_guard<std::mutex> lock(this->event_queue_mutex);
    this->event_queue.pop();
}