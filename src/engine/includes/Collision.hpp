#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "EventHandler.hpp"

class Collision : public Component, public EventHandler {
  private:
    Entity *entity;
    float restitution;
    bool avoid_transform;

    void HandlePairwiseCollision(Entity *collider);

  public:
    Collision(Entity *entity);

    float GetRestitution();
    bool GetAvoidTransform();

    void SetRestitution(float restitution);
    void SetAvoidTransform(bool avoid_transform);

    void Update() override;
    void OnEvent(Event event) override;
};