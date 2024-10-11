#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include <unordered_set>

class Collision : public Component {
  private:
    Entity *entity;
    float restitution;
    std::unordered_set<Entity *> colliders;

  public:
    Collision(Entity *entity);

    float GetRestitution();
    void SetRestitution(float restitution);

    std::unordered_set<Entity *> GetColliders();
    void AddCollider(Entity *entity);
    void RemoveCollider(Entity *entity);

    void HandleCollisions();

    void Update() override;
};