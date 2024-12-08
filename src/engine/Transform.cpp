#include "Transform.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Event.hpp"
#include "EventManager.hpp"
#include "Render.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <SDL_rect.h>

#include "Profile.hpp"
PROFILED;

Transform::Transform(Entity *entity) {
    this->entity = entity;
    this->SetPosition(Position{0, 0});
    this->size = Size{0, 0};
    this->angle = 0;
    this->anchor = SDL_Point{0, 0};

    EventManager::GetInstance().Register({EventType::Move, EventType::Spawn}, this);
}

Position Transform::GetPosition() {
    std::lock_guard<std::mutex> lock(this->position_mutex);
    return this->position;
}
Size Transform::GetSize() { return this->size; }
double Transform::GetAngle() {
    std::lock_guard<std::mutex> lock(this->angle_mutex);
    return this->angle;
}
SDL_Point Transform::GetAnchor() { return this->anchor; }

void Transform::SetPosition(Position position) {
    ZoneScoped;

    std::string zone_text = this->entity->GetName() + " x: " + std::to_string(position.x) +
                            " y: " + std::to_string(position.y);
    ZoneText(zone_text.c_str(), zone_text.size());

    std::lock_guard<std::mutex> lock(this->position_mutex);
    this->position = position;
}
void Transform::SetSize(Size size) { this->size = size; }
void Transform::SetAngle(double angle) {
    std::lock_guard<std::mutex> lock(this->angle_mutex);
    this->angle = angle;
}
void Transform::SetAnchor(SDL_Point anchor) { this->anchor = anchor; }

void Transform::Update() {};

void Transform::OnEvent(Event event) {
    EventType event_type = event.type;

    switch (event_type) {
    case EventType::Move: {
        MoveEvent *move_event = std::get_if<MoveEvent>(&(event.data));
        if (move_event) {
            if (this->entity == move_event->entity) {
                this->SetPosition(move_event->position);
                this->SetAngle(move_event->angle);
                EventManager::GetInstance().RaiseSendUpdateEvent(SendUpdateEvent{this->entity});
            }
        }

        break;
    }

    case EventType::Spawn: {
        SpawnEvent *spawn_event = std::get_if<SpawnEvent>(&(event.data));
        if (spawn_event) {
            if (this->entity == GetClientPlayer(Engine::GetInstance().GetNetworkInfo().id,
                                                Engine::GetInstance().GetEntities())) {
                if (this->entity == spawn_event->entity) {
                    this->entity->GetComponent<Render>()->SetVisible(true);
                    Engine::GetInstance().RespawnPlayer();
                }
            }
        }

        break;
    }

    default:
        break;
    }
}