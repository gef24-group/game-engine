#pragma once
#include "Component.hpp"
#include "Types.hpp"

class NetworkComponent : Component {
    std::string player_address;
    NetworkRole owner;

  public:
    NetworkComponent();
    std::string GetPlayerAddress();
    NetworkRole GetOwner();

    void SetPlayerAddress(std::string player_address);
    void SetOwner(NetworkRole owner);

    void Update(GameObject *game_object, int64_t delta);
};