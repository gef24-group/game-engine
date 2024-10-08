#pragma once
#include "GameObject.hpp"
#include <unordered_set>

class CollisionComponent : Component {
  private:
    std::unordered_set<GameObject *> colliders;
    bool affected_by_collision;
    float restitution;

  public:
    float GetRestitution();
    bool GetAffectedByCollision();

    void SetAffectedByCollision(bool affected_by_collision);
    void SetRestitution(float restitution);

    std::unordered_set<GameObject *> GetColliders();
    void AddCollider(GameObject *game_object);
    void RemoveCollider(GameObject *game_object);

    void HandleCollisions();

    void Update(GameObject *game_object);
};