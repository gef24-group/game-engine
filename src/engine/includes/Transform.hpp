#pragma once

#include "Component.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include <mutex>

class Transform : public Component {
  private:
    GameObject *game_object;
    std::mutex position_mutex;
    Position position;
    Size size;

  public:
    Transform(GameObject *game_object);

    Position GetPosition();
    Size GetSize();

    void SetPosition(Position position);
    void SetSize(Size size);

    void Update() override;
};