#pragma once

#include "Component.hpp"
#include "GameObject.hpp"

class Handler : public Component {
  private:
    GameObject *game_object;
    std::function<void(GameObject *)> callback;

  public:
    Handler(GameObject *game_object);

    std::function<void(GameObject *)> GetCallback();
    // The callback references the function that makes the object react to inputs
    void SetCallback(std::function<void(GameObject *)> callback);
    void Update() override;
};