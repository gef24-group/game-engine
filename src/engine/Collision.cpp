#include "Collision.hpp"
#include "Entity.hpp"
#include "Physics.hpp"
#include "Transform.hpp"

Collision::Collision(Entity *entity) {
    this->entity = entity;
    this->affected_by_collision = true;
    this->restitution = 0;
    this->colliders = std::unordered_set<Entity *>();
}

bool Collision::GetAffectedByCollision() { return this->affected_by_collision; }
float Collision::GetRestitution() { return this->restitution; }

void Collision::SetAffectedByCollision(bool affected_by_collision) {
    this->affected_by_collision = affected_by_collision;
}
void Collision::SetRestitution(float restitution) { this->restitution = restitution; }

std::unordered_set<Entity *> Collision::GetColliders() { return this->colliders; }
void Collision::AddCollider(Entity *entity) { this->colliders.insert(entity); }
void Collision::RemoveCollider(Entity *entity) { this->colliders.erase(entity); }

// Multithreading for this will have to be handled separately
void Collision::HandleCollisions() {
    if (!this->affected_by_collision) {
        return;
    }

    if (this->GetColliders().size() > 0) {
        for (Entity *collider : this->GetColliders()) {
            int obj_x = static_cast<int>(
                std::round(this->entity->GetComponent<Transform>()->GetPosition().x));
            int obj_y = static_cast<int>(
                std::round(this->entity->GetComponent<Transform>()->GetPosition().y));

            int col_x =
                static_cast<int>(std::round(collider->GetComponent<Transform>()->GetPosition().x));
            int col_y =
                static_cast<int>(std::round(collider->GetComponent<Transform>()->GetPosition().y));

            int obj_width = this->entity->GetComponent<Transform>()->GetSize().width;
            int obj_height = this->entity->GetComponent<Transform>()->GetSize().height;
            int col_width = collider->GetComponent<Transform>()->GetSize().width;
            int col_height = collider->GetComponent<Transform>()->GetSize().height;

            int left_overlap = (obj_x + obj_width) - col_x;
            int right_overlap = (col_x + col_width) - obj_x;
            int top_overlap = (obj_y + obj_height) - col_y;
            int bottom_overlap = (col_y + col_height) - obj_y;

            int min_overlap = std::min(std::min(left_overlap, right_overlap),
                                       std::min(top_overlap, bottom_overlap));

            int pos_x = 0, pos_y = 0;
            if (min_overlap == left_overlap) {
                pos_x = col_x - obj_width;
                pos_y = obj_y;
            } else if (min_overlap == right_overlap) {
                pos_x = col_x + col_width;
                pos_y = obj_y;
            } else if (min_overlap == top_overlap) {
                pos_x = obj_x;
                pos_y = col_y - obj_height;
            } else if (min_overlap == bottom_overlap) {
                pos_x = obj_x;
                pos_y = col_y + col_height;
            }

            this->entity->GetComponent<Transform>()->SetPosition(
                Position{float(pos_x), float(pos_y)});

            float vel_x = this->entity->GetComponent<Physics>()->GetVelocity().x;
            float vel_y = this->entity->GetComponent<Physics>()->GetVelocity().y;
            if (min_overlap == left_overlap || min_overlap == right_overlap) {
                vel_x *= -this->GetRestitution();
            }
            if (min_overlap == top_overlap || min_overlap == bottom_overlap) {
                vel_y *= -this->GetRestitution();
            }
            this->entity->GetComponent<Physics>()->SetVelocity(Velocity{vel_x, vel_y});
        }
    }
}

void Collision::Update() { this->HandleCollisions(); }