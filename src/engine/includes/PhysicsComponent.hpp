#include "GameObject.hpp"
#include "Types.hpp"
#include <mutex>

class PhysicsComponent : Component {
  private:
    GameObject *game_object;
    std::mutex position_mutex;
    Position position;
    Velocity velocity;
    Acceleration acceleration;
    std::function<void(GameObject *)> callback;

  public:
    PhysicsComponent();
    Position GetPosition();
    Velocity GetVelocity();
    Acceleration GetAcceleration();

    void SetPosition(Position position);
    void SetVelocity(Velocity velocity);
    void SetAcceleration(Acceleration acceleration);
    void Move(int64_t delta);
    void ReactToPlayerInput();

    // The callback references the function that makes the object react to inputs
    std::function<void(GameObject *)> GetCallback();
    void SetCallback(std::function<void(GameObject *)> callback);

    void Update(GameObject *game_object);
};