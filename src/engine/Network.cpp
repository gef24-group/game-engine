#include "Network.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Event.hpp"
#include "EventManager.hpp"
#include "Types.hpp"
#include "Utils.hpp"

Network::Network(Entity *entity) {
    EventManager::GetInstance().Register({EventType::Move, EventType::Spawn}, this);

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

void Network::OnEvent(Event event) {
    EventType event_type = event.type;

    switch (event_type) {
    case EventType::Move: {
        MoveEvent *move_event = std::get_if<MoveEvent>(&(event.data));
        if (move_event && move_event->entity_name == this->entity->GetName()) {
            if (Engine::GetInstance().GetNetworkInfo().role == NetworkRole::Server) {
                Engine::GetInstance().CSServerBroadcastUpdates(this->entity);
            } else if (Engine::GetInstance().GetNetworkInfo().role == NetworkRole::Client) {
                if (this->entity == GetClientPlayer(Engine::GetInstance().GetNetworkInfo().id,
                                                    Engine::GetInstance().GetEntities())) {
                    Engine::GetInstance().CSClientSendUpdate();
                }
            } else {
                Log(LogLevel::Error, "Network mode/role not supported");
            }
        } else {
        }

        break;
    }
    default:
        break;
    }
}