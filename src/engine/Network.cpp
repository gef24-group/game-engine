#include "Network.hpp"
#include "GameObject.hpp"

Network::Network(GameObject *game_object) {
    this->game_object = game_object;
    this->player_address = "";
    this->owner = NetworkRole::Client;
}

std::string Network::GetPlayerAddress() { return this->player_address; }
NetworkRole Network::GetOwner() { return this->owner; }

void Network::SetPlayerAddress(std::string player_address) {
    this->player_address = player_address;
}
void Network::SetOwner(NetworkRole owner) { this->owner = owner; }

void Network::Update() {}