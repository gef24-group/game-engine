#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "EventHandler.hpp"

class Collision : public Component, public EventHandler {
  private:
    Entity *entity;
    float restitution;

    void HandlePairwiseCollision(Entity *collider);

  public:
    Collision(Entity *entity);

    float GetRestitution();
    void SetRestitution(float restitution);

    void Update() override;
    void OnEvent(Event event) override;
};