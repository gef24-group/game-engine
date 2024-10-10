#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "Timeline.hpp"
#include "Types.hpp"

class Physics : public Component {
  private:
    Entity *entity;
    std::shared_ptr<Timeline> engine_timeline;

    Velocity velocity;
    Acceleration acceleration;

  public:
    Physics(Entity *entity);

    void SetEngineTimeline(std::shared_ptr<Timeline> engine_timeline);

    Velocity GetVelocity();
    Acceleration GetAcceleration();

    void SetVelocity(Velocity velocity);
    void SetAcceleration(Acceleration acceleration);

    void Move();
    void Update() override;
};