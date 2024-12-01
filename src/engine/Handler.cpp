#include "Handler.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "EventManager.hpp"
#include "Utils.hpp"
#include <vector>

Handler::Handler(Entity *entity) {
    this->entity = entity;
    this->update_callback = [](Entity &) {};
    this->event_callback = [](Entity &, Event &) {};

    EventManager::GetInstance().Register({EventType::Input, EventType::Collision}, this);
}

std::function<void(Entity &)> Handler::GetUpdateCallback() { return this->update_callback; }

void Handler::SetUpdateCallback(std::function<void(Entity &)> update_callback) {
    this->update_callback = update_callback;
}

void Handler::SetEventCallback(std::function<void(Entity &, Event &)> event_callback) {
    // the callback contains the reaction to keyboard inputs
    this->event_callback = event_callback;
}

void Handler::Update() { this->update_callback(*this->entity); }

void Handler::OnEvent(Event event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));
    int player_id = Engine::GetInstance().GetNetworkInfo().id;
    std::vector<Entity *> entities = Engine::GetInstance().GetEntities();

    if (input_event && this->entity != GetClientPlayer(player_id, entities)) {
        // Return early to avoid triggering input events for objects that inputs aren't meant for
        return;
    }

    this->event_callback(*this->entity, event);
}