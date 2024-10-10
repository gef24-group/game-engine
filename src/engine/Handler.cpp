#include "Handler.hpp"
#include "Entity.hpp"

Handler::Handler(Entity *entity) {
    this->entity = entity;
    this->callback = [](Entity *) {};
}

std::function<void(Entity *)> Handler::GetCallback() { return this->callback; }

void Handler::SetCallback(std::function<void(Entity *)> callback) { this->callback = callback; }

void Handler::Update() {
    // the callback contains the reaction to keyboard inputs
    this->callback(this->entity);
}