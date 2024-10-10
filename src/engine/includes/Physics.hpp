#pragma once

#include "Component.hpp"
#include "GameObject.hpp"
#include "Timeline.hpp"
#include "Types.hpp"

class Physics : public Component {
  private:
    GameObject *game_object;
    std::shared_ptr<Timeline> engine_timeline;

    Velocity velocity;
    Acceleration acceleration;

  public:
    Physics(GameObject *game_object);

    void SetEngineTimeline(std::shared_ptr<Timeline> engine_timeline);

    Velocity GetVelocity();
    Acceleration GetAcceleration();

    void SetVelocity(Velocity velocity);
    void SetAcceleration(Acceleration acceleration);

    void Move();
    void Update() override;
};