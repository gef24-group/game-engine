#include "Collision.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Event.hpp"
#include "EventManager.hpp"
#include "Network.hpp"
#include "Physics.hpp"
#include "Render.hpp"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <variant>

#include "Profile.hpp"
PROFILED;

Collision::Collision(Entity *entity) {
    this->entity = entity;
    this->restitution = 0;

    EventManager::GetInstance().Register({EventType::Collision, EventType::Death}, this);
}

float Collision::GetRestitution() { return this->restitution; }
void Collision::SetRestitution(float restitution) { this->restitution = restitution; }

void Collision::HandlePairwiseCollision(Entity *collider) {
    ZoneScoped;

    if (collider == nullptr) {
        return;
    }

    if (collider->GetCategory() == EntityCategory::SpawnPoint) {
        return;
    }

    if (this->entity == GetClientPlayer(Engine::GetInstance().GetNetworkInfo().id,
                                        Engine::GetInstance().GetEntities())) {
        if (collider->GetCategory() == EntityCategory::DeathZone) {
            EventManager::GetInstance().RaiseDeathEvent(DeathEvent{this->entity});
            return;
        }
        if (collider->GetCategory() == EntityCategory::SideBoundary) {
            Engine::GetInstance().HandleSideBoundaries(collider);
        }
    }

    if (this->entity->GetComponent<Network>() != nullptr) {
        NetworkRole entity_owner = this->entity->GetComponent<Network>()->GetOwner();
        NetworkRole engine_role = Engine::GetInstance().GetNetworkInfo().role;

        if (entity_owner != engine_role) {
            return;
        }
    }

    int obj_x =
        static_cast<int>(std::round(this->entity->GetComponent<Transform>()->GetPosition().x));
    int obj_y =
        static_cast<int>(std::round(this->entity->GetComponent<Transform>()->GetPosition().y));
    int col_x = static_cast<int>(std::round(collider->GetComponent<Transform>()->GetPosition().x));
    int col_y = static_cast<int>(std::round(collider->GetComponent<Transform>()->GetPosition().y));

    int obj_width = this->entity->GetComponent<Transform>()->GetSize().width;
    int obj_height = this->entity->GetComponent<Transform>()->GetSize().height;
    int col_width = collider->GetComponent<Transform>()->GetSize().width;
    int col_height = collider->GetComponent<Transform>()->GetSize().height;

    SDL_Rect rect_1 = {obj_x, obj_y, obj_width, obj_height};
    SDL_Rect rect_2 = {col_x, col_y, col_width, col_height};
    Overlap overlap = GetOverlap(rect_1, rect_2);

    int pos_x = 0, pos_y = 0;

    if (overlap == Overlap::Left) {
        pos_x = col_x - obj_width;
        pos_y = obj_y;
    } else if (overlap == Overlap::Right) {
        pos_x = col_x + col_width;
        pos_y = obj_y;
    } else if (overlap == Overlap::Top) {
        pos_x = obj_x;
        pos_y = col_y - obj_height;
    } else if (overlap == Overlap::Bottom) {
        pos_x = obj_x;
        pos_y = col_y + col_height;
    }

    if (this->entity->GetComponent<Physics>()) {
        this->entity->GetComponent<Transform>()->SetPosition(Position{float(pos_x), float(pos_y)});
        float vel_x = this->entity->GetComponent<Physics>()->GetVelocity().x;
        float vel_y = this->entity->GetComponent<Physics>()->GetVelocity().y;

        if (collider->GetCategory() != EntityCategory::SideBoundary) {
            if (overlap == Overlap::Left || overlap == Overlap::Right) {
                vel_x *= -this->GetRestitution();
            }
            if (overlap == Overlap::Top || overlap == Overlap::Bottom) {
                vel_y *= -this->GetRestitution();
            }
        }

        this->entity->GetComponent<Physics>()->SetVelocity(Velocity{vel_x, vel_y});
    }
}

void Collision::Update() {}

void Collision::OnEvent(Event event) {
    EventType event_type = event.type;

    switch (event_type) {
    case EventType::Collision: {
        CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));
        Entity *collider = nullptr;
        if (collision_event) {
            if (this->entity == collision_event->collider_1) {
                collider = collision_event->collider_2;
            } else if (this->entity == collision_event->collider_2) {
                collider = collision_event->collider_1;
            }
            this->HandlePairwiseCollision(collider);
        }

        break;
    }

    case EventType::Death: {
        DeathEvent *death_event = std::get_if<DeathEvent>(&(event.data));
        if (death_event) {
            if (this->entity == GetClientPlayer(Engine::GetInstance().GetNetworkInfo().id,
                                                Engine::GetInstance().GetEntities())) {
                if (this->entity == death_event->entity) {
                    this->entity->GetComponent<Render>()->SetVisible(false);
                    EventManager::GetInstance().RaiseSpawnEvent(SpawnEvent{this->entity});
                }
            }
        }

        break;
    }

    default:
        break;
    }
}