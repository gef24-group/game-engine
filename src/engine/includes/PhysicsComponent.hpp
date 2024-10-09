#pragma once
#include "Component.hpp"
#include "Types.hpp"
#include <mutex>

class PhysicsComponent : Component {
  private:
    GameObject *game_object;
    std::mutex position_mutex;
    Position position;
    Velocity velocity;
    Acceleration acceleration;
    Size size;
    std::function<void(GameObject *)> callback;

  public:
    PhysicsComponent(GameObject *game_object);
    Position GetPosition();
    Velocity GetVelocity();
    Acceleration GetAcceleration();
    Size GetSize();

    void SetPosition(Position position);
    void SetVelocity(Velocity velocity);
    void SetAcceleration(Acceleration acceleration);
    void SetSize(Size size);

    void Move(int64_t delta);
    // void ReactToPlayerInput(GameObject *game_object);

    // The callback references the function that makes the object react to inputs
    std::function<void(GameObject *)> GetCallback();
    void SetCallback(std::function<void(GameObject *)> callback);

    // update calls both Move and ReactToPlayerInput
    void Update(GameObject *game_object, int64_t delta);
};