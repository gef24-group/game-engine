#include "Transform.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Event.hpp"
#include "EventManager.hpp"
#include "Render.hpp"
#include "Types.hpp"
#include "Utils.hpp"

#include "Profile.hpp"
PROFILED;

Transform::Transform(Entity *entity) {
    EventManager::GetInstance().Register({EventType::Move, EventType::Spawn}, this);

    this->entity = entity;
    this->SetPosition(Position{0, 0});
    this->size = Size{0, 0};
}

Position Transform::GetPosition() {
    std::lock_guard<std::mutex> lock(this->position_mutex);
    return this->position;
}
Size Transform::GetSize() { return this->size; }

void Transform::SetPosition(Position position) {
    ZoneScoped;

    std::string zone_text = this->entity->GetName() + " x: " + std::to_string(position.x) +
                            " y: " + std::to_string(position.y);
    ZoneText(zone_text.c_str(), zone_text.size());

    std::lock_guard<std::mutex> lock(this->position_mutex);
    this->position = position;
}
void Transform::SetSize(Size size) { this->size = size; }

void Transform::Update() {};

void Transform::OnEvent(Event event) {
    EventType event_type = event.type;

    switch (event_type) {
    case EventType::Move: {
        MoveEvent *move_event = std::get_if<MoveEvent>(&(event.data));
        if (move_event) {
            if (this->entity->GetName() == move_event->entity_name) {
                this->SetPosition(move_event->position);
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