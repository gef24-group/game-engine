#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "EventHandler.hpp"
#include "Types.hpp"
#include <mutex>

class Transform : public Component, public EventHandler {
  private:
    Entity *entity;
    std::mutex position_mutex;
    Position position;
    Size size;
    std::mutex angle_mutex;
    double angle;
    SDL_Point anchor;

  public:
    Transform(Entity *entity);

    Position GetPosition();
    Size GetSize();
    double GetAngle();
    SDL_Point GetAnchor();

    void SetPosition(Position position);
    void SetSize(Size size);
    void SetAngle(double angle);
    void SetAnchor(SDL_Point anchor);

    void Update() override;
    void OnEvent(Event event) override;
};