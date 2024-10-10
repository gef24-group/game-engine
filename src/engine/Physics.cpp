#include "Physics.hpp"
#include "GameObject.hpp"
#include "Timeline.hpp"
#include "Transform.hpp"
#include <cmath>
#include <cstdint>

Physics::Physics(GameObject *game_object) {
    this->game_object = game_object;
    this->engine_timeline = std::make_shared<Timeline>();
    this->velocity = Velocity{0, 0};
    this->acceleration = Acceleration{0, 0};
}

void Physics::SetEngineTimeline(std::shared_ptr<Timeline> engine_timeline) {
    this->engine_timeline = engine_timeline;
}

Velocity Physics::GetVelocity() { return this->velocity; }
Acceleration Physics::GetAcceleration() { return this->acceleration; }

void Physics::SetVelocity(Velocity velocity) {
    // maybe add a reference to the game object to deal with this
    if (this->game_object->GetCategory() != Stationary) {
        this->velocity = velocity;
    }
}
void Physics::SetAcceleration(Acceleration acceleration) {
    // maybe add a reference to the game object to deal with this
    if (this->game_object->GetCategory() != Stationary) {
        this->acceleration = acceleration;
    }
}

void Physics::Move() {
    int64_t delta = this->engine_timeline->GetFrameTime().delta;

    const float HALF = 0.5;
    float time = static_cast<float>(delta) / 100'000'000.0f;
    Position curr_position = this->game_object->GetComponent<Transform>()->GetPosition();
    float new_pos_x = curr_position.x + (this->velocity.x * time) +
                      (HALF * this->acceleration.x * float(pow(time, 2)));
    float new_pos_y = curr_position.y + (this->velocity.y * time) +
                      (HALF * this->acceleration.y * float(pow(time, 2)));

    this->game_object->GetComponent<Transform>()->SetPosition(Position{new_pos_x, new_pos_y});

    this->velocity.x += (this->acceleration.x * time);
    this->velocity.y += (this->acceleration.y * time);
}

void Physics::Update() {
    // Run the Move function only if engine_timeline is not null
    if (this->engine_timeline) {
        this->Move();
    }
}