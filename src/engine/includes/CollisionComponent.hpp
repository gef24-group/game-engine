#pragma once
#include "Component.hpp"
#include <cstdint>
#include <unordered_set>

class CollisionComponent : Component {
  private:
    std::unordered_set<GameObject *> colliders;
    bool affected_by_collision;
    float restitution;

  public:
    CollisionComponent();
    float GetRestitution();
    bool GetAffectedByCollision();

    void SetAffectedByCollision(bool affected_by_collision);
    void SetRestitution(float restitution);

    std::unordered_set<GameObject *> GetColliders();
    void AddCollider(GameObject *game_object);
    void RemoveCollider(GameObject *game_object);

    void HandleCollisions(GameObject *game_object);

    void Update(GameObject *game_object, int64_t delta);
};