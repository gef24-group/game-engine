#include "Collision.hpp"
#include "Entity.hpp"
#include "Physics.hpp"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"

Collision::Collision(Entity *entity) {
    this->entity = entity;
    this->restitution = 0;
    this->colliders = std::unordered_set<Entity *>();
}

float Collision::GetRestitution() { return this->restitution; }
void Collision::SetRestitution(float restitution) { this->restitution = restitution; }

std::unordered_set<Entity *> Collision::GetColliders() { return this->colliders; }
void Collision::AddCollider(Entity *entity) { this->colliders.insert(entity); }
void Collision::RemoveCollider(Entity *entity) { this->colliders.erase(entity); }

// Multithreading for this will have to be handled separately
void Collision::HandleCollisions() {
    for (Entity *collider : this->GetColliders()) {
        if (collider->GetCategory() == EntityCategory::SpawnPoint) {
            return;
        }

        int obj_x =
            static_cast<int>(std::round(this->entity->GetComponent<Transform>()->GetPosition().x));
        int obj_y =
            static_cast<int>(std::round(this->entity->GetComponent<Transform>()->GetPosition().y));
        int col_x =
            static_cast<int>(std::round(collider->GetComponent<Transform>()->GetPosition().x));
        int col_y =
            static_cast<int>(std::round(collider->GetComponent<Transform>()->GetPosition().y));

        int obj_width = this->entity->GetComponent<Transform>()->GetSize().width;
        int obj_height = this->entity->GetComponent<Transform>()->GetSize().height;
        int col_width = collider->GetComponent<Transform>()->GetSize().width;
        int col_height = collider->GetComponent<Transform>()->GetSize().height;

        SDL_Rect rect_1 = {obj_x, obj_y, obj_width, obj_height};
        SDL_Rect rect_2 = {col_x, col_y, col_width, col_height};
        Overlap overlap = GetOverlap(rect_1, rect_2);

        int pos_x = 0, pos_y = 0;
        float vel_x = this->entity->GetComponent<Physics>()->GetVelocity().x;
        float vel_y = this->entity->GetComponent<Physics>()->GetVelocity().y;

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

        this->entity->GetComponent<Transform>()->SetPosition(Position{float(pos_x), float(pos_y)});

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

void Collision::Update() { this->HandleCollisions(); }