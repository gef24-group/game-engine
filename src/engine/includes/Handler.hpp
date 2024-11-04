#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "EventHandler.hpp"
#include <functional>

class Handler : public Component, public EventHandler {
  private:
    Entity *entity;
    std::function<void(Entity &)> update_callback;
    std::function<void(Entity &, Event &)> event_callback;

  public:
    Handler(Entity *entity);

    std::function<void(Entity &)> GetUpdateCallback();
    void SetUpdateCallback(std::function<void(Entity &)> update_callback);
    // The callback references the function that makes the entity react to inputs
    void SetEventCallback(std::function<void(Entity &, Event &)> event_callback);

    void Update() override;
    void OnEvent(Event event) override;
};