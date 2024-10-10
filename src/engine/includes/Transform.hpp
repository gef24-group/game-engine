#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "Types.hpp"
#include <mutex>

class Transform : public Component {
  private:
    Entity *entity;
    std::mutex position_mutex;
    Position position;
    Size size;

  public:
    Transform(Entity *entity);

    Position GetPosition();
    Size GetSize();

    void SetPosition(Position position);
    void SetSize(Size size);

    void Update() override;
};