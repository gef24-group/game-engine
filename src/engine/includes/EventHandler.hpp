#pragma once

#include "Event.hpp"

class EventHandler {
  public:
    virtual ~EventHandler() = default;
    virtual void OnEvent(Event event) = 0;
};