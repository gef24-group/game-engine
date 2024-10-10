#include "Handler.hpp"
#include "GameObject.hpp"

Handler::Handler(GameObject *game_object) {
    this->game_object = game_object;
    this->callback = [](GameObject *) {};
}

std::function<void(GameObject *)> Handler::GetCallback() { return this->callback; }

void Handler::SetCallback(std::function<void(GameObject *)> callback) { this->callback = callback; }

void Handler::Update() {
    // the callback contains the reaction to keyboard inputs
    this->callback(this->game_object);
}