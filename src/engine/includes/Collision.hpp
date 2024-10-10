#pragma once

#include "Component.hpp"
#include "GameObject.hpp"
#include <unordered_set>

class Collision : public Component {
  private:
    GameObject *game_object;
    bool affected_by_collision;
    float restitution;
    std::unordered_set<GameObject *> colliders;

  public:
    Collision(GameObject *game_object);

    bool GetAffectedByCollision();
    float GetRestitution();

    void SetAffectedByCollision(bool affected_by_collision);
    void SetRestitution(float restitution);

    std::unordered_set<GameObject *> GetColliders();
    void AddCollider(GameObject *game_object);
    void RemoveCollider(GameObject *game_object);

    void HandleCollisions();

    void Update() override;
};