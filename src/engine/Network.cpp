#include "Network.hpp"
#include "Entity.hpp"

Network::Network(Entity *entity) {
    this->entity = entity;
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