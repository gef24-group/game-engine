#include "CollisionComponent.hpp"
#include "GameObject.hpp"
#include "PhysicsComponent.hpp"

CollisionComponent::CollisionComponent() {
    this->affected_by_collision = true;
    this->restitution = 0;
    this->colliders = std::unordered_set<GameObject *>();
}

bool CollisionComponent::GetAffectedByCollision() { return this->affected_by_collision; }
float CollisionComponent::GetRestitution() { return this->restitution; }

void CollisionComponent::SetAffectedByCollision(bool affected_by_collision) {
    this->affected_by_collision = affected_by_collision;
}
void CollisionComponent::SetRestitution(float restitution) { this->restitution = restitution; }

std::unordered_set<GameObject *> CollisionComponent::GetColliders() { return this->colliders; }
void CollisionComponent::AddCollider(GameObject *game_object) {
    this->colliders.insert(game_object);
}
void CollisionComponent::RemoveCollider(GameObject *game_object) {
    this->colliders.erase(game_object);
}

// Multithreading for this will have to be handled separately
void CollisionComponent::HandleCollisions(GameObject *game_object) {
    if (this->GetColliders().size() > 0) {
        for (GameObject *collider : this->GetColliders()) {
            int obj_x = static_cast<int>(
                std::round(game_object->GetComponent<PhysicsComponent>()->GetPosition().x));
            int obj_y = static_cast<int>(
                std::round(game_object->GetComponent<PhysicsComponent>()->GetPosition().y));

            int col_x = static_cast<int>(
                std::round(collider->GetComponent<PhysicsComponent>()->GetPosition().x));
            int col_y = static_cast<int>(
                std::round(collider->GetComponent<PhysicsComponent>()->GetPosition().y));

            int obj_width = game_object->GetComponent<PhysicsComponent>()->GetSize().width;
            int obj_height = game_object->GetComponent<PhysicsComponent>()->GetSize().height;
            int col_width = collider->GetComponent<PhysicsComponent>()->GetSize().width;
            int col_height = collider->GetComponent<PhysicsComponent>()->GetSize().height;

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

            game_object->GetComponent<PhysicsComponent>()->SetPosition(
                Position{float(pos_x), float(pos_y)});

            float vel_x = game_object->GetComponent<PhysicsComponent>()->GetVelocity().x;
            float vel_y = game_object->GetComponent<PhysicsComponent>()->GetVelocity().y;
            if (min_overlap == left_overlap || min_overlap == right_overlap) {
                vel_x *= -this->GetRestitution();
            }
            if (min_overlap == top_overlap || min_overlap == bottom_overlap) {
                vel_y *= -this->GetRestitution();
            }
            game_object->GetComponent<PhysicsComponent>()->SetVelocity(Velocity{vel_x, vel_y});
        }
    }
}

void CollisionComponent::Update(GameObject *game_object, int64_t delta) {
    this->HandleCollisions(game_object);
}