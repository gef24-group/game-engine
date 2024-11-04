#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "EventHandler.hpp"
#include <unordered_set>

class Collision : public Component, public EventHandler {
  private:
    Entity *entity;
    float restitution;
    std::unordered_set<Entity *> colliders;

    void HandlePairwiseCollision(Entity *collider);

  public:
    Collision(Entity *entity);

    float GetRestitution();
    void SetRestitution(float restitution);

    std::unordered_set<Entity *> GetColliders();
    void AddCollider(Entity *entity);
    void RemoveCollider(Entity *entity);

    void Update() override;
    void OnEvent(Event event) override;
};