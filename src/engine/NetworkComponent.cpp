#include "NetworkComponent.hpp"
#include "GameObject.hpp"

void NetworkComponent::Update(GameObject *game_object, int64_t delta) {}

std::string NetworkComponent::GetPlayerAddress() { return this->player_address; }
NetworkRole NetworkComponent::GetOwner() { return this->owner; }

void NetworkComponent::SetPlayerAddress(std::string player_address) {
    this->player_address = player_address;
}
void NetworkComponent::SetOwner(NetworkRole owner) { this->owner = owner; }