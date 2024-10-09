#include "PhysicsComponent.hpp"
#include "GameObject.hpp"
#include <cmath>
#include <cstdint>

PhysicsComponent::PhysicsComponent(GameObject *game_object) {
    this->game_object = game_object;
    this->SetPosition(Position{0, 0});
    this->velocity = Velocity{0, 0};
    this->acceleration = Acceleration{0, 0};
    this->callback = [](GameObject *) {};
}

Position PhysicsComponent::GetPosition() {
    std::lock_guard<std::mutex> lock(this->position_mutex);
    return this->position;
}
Velocity PhysicsComponent::GetVelocity() { return this->velocity; }
Acceleration PhysicsComponent::GetAcceleration() { return this->acceleration; }
std::function<void(GameObject *)> PhysicsComponent::GetCallback() { return this->callback; }
Size PhysicsComponent::GetSize() { return this->size; }

void PhysicsComponent::SetPosition(Position position) {
    std::lock_guard<std::mutex> lock(this->position_mutex);
    this->position = position;
}

void PhysicsComponent::SetVelocity(Velocity velocity) {
    // maybe add a reference to the game object to deal with this
    if (this->game_object->GetCategory() != Stationary) {
        this->velocity = velocity;
    }
}

void PhysicsComponent::SetAcceleration(Acceleration acceleration) {
    // maybe add a reference to the game object to deal with this
    if (this->game_object->GetCategory() != Stationary) {
        this->acceleration = acceleration;
    }
}

void PhysicsComponent::SetSize(Size size) { this->size = size; }

void PhysicsComponent::Move(int64_t delta) {
    const float HALF = 0.5;
    float time = static_cast<float>(delta) / 100'000'000.0f;
    Position curr_position = this->GetPosition();
    float new_pos_x = curr_position.x + (this->velocity.x * time) +
                      (HALF * this->acceleration.x * float(pow(time, 2)));
    float new_pos_y = curr_position.y + (this->velocity.y * time) +
                      (HALF * this->acceleration.y * float(pow(time, 2)));

    this->SetPosition(Position{new_pos_x, new_pos_y});

    this->velocity.x += (this->acceleration.x * time);
    this->velocity.y += (this->acceleration.y * time);
}

void PhysicsComponent::SetCallback(std::function<void(GameObject *)> callback) {
    this->callback = callback;
}

void PhysicsComponent::Update(GameObject *game_object, int64_t delta) {
    // the callback contains the reaction to keyboard inputs
    this->callback(game_object);
    this->Move(delta);
}