#include "Network.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Event.hpp"
#include "EventManager.hpp"
#include "Replay.hpp"
#include "Types.hpp"
#include "Utils.hpp"

Network::Network(Entity *entity) {
    this->entity = entity;
    this->active = true;
    this->player_address = "";
    this->owner = NetworkRole::Client;

    EventManager::GetInstance().Register({EventType::Move}, this);
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
        if (Replay::GetInstance().GetIsReplaying()) {
            return;
        }

        MoveEvent *move_event = std::get_if<MoveEvent>(&(event.data));
        if (move_event && move_event->entity == this->entity) {
            NetworkRole engine_role = Engine::GetInstance().GetNetworkInfo().role;

            switch (engine_role) {
            case NetworkRole::Server:
                Engine::GetInstance().CSServerBroadcastUpdates(this->entity);
                break;
            case NetworkRole::Client:
                if (this->entity == GetClientPlayer(Engine::GetInstance().GetNetworkInfo().id,
                                                    Engine::GetInstance().GetEntities())) {
                    Engine::GetInstance().CSClientSendUpdate();
                }
                break;
            case NetworkRole::Host:
            case NetworkRole::Peer:
                if (this->entity->GetComponent<Network>() &&
                    this->entity->GetComponent<Network>()->GetOwner() == engine_role) {
                    Engine::GetInstance().P2PBroadcastUpdates(this->entity);
                }
                break;
            default:
                Log(LogLevel::Error, "Network mode/role not supported");
                break;
            }
        }
        break;
    }
    default:
        break;
    }
}