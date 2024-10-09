
#pragma once
#include <string>

class GameObject;

class Component {
  private:
    std::string type;

  public:
    virtual ~Component() = default;
    virtual void Update(GameObject *game_object, int64_t delta) = 0;
};