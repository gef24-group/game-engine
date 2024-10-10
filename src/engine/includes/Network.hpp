#pragma once

#include "Component.hpp"
#include "GameObject.hpp"
#include "Types.hpp"

class Network : public Component {
    GameObject *game_object;
    std::string player_address;
    NetworkRole owner;

  public:
    Network(GameObject *game_object);

    std::string GetPlayerAddress();
    NetworkRole GetOwner();

    void SetPlayerAddress(std::string player_address);
    void SetOwner(NetworkRole owner);

    void Update() override;
};