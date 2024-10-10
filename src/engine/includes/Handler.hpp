#pragma once

#include "Component.hpp"
#include "Entity.hpp"

class Handler : public Component {
  private:
    Entity *entity;
    std::function<void(Entity *)> callback;

  public:
    Handler(Entity *entity);

    std::function<void(Entity *)> GetCallback();
    // The callback references the function that makes the entity react to inputs
    void SetCallback(std::function<void(Entity *)> callback);
    void Update() override;
};