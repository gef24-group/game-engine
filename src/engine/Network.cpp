#include "Network.hpp"
#include "Entity.hpp"

Network::Network(Entity *entity) {
    this->entity = entity;
    this->active = true;
    this->player_address = "";
    this->owner = NetworkRole::Client;
}

bool Network::GetActive() { return this->active.load(); }
std::string Network::GetPlayerAddress() { return this->player_address; }
NetworkRole Network::GetOwner() { return this->owner; }

void Network::SetActive(bool active) { this->active.store(active); }
void Network::SetPlayerAddress(std::string player_address) {
    this->player_address = player_address;
}
void Network::SetOwner(NetworkRole owner) { this->owner = owner; }

void Network::Update() {}