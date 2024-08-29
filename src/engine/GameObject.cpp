#include "GameObject.hpp"
#include "Constants.hpp"
#include <cmath>
#include <functional>
#include <utility>

GameObject::GameObject(GameObjectCategory object_category)
    : shape(Rectangle), red(DEFAULT_OBJECT_RED), green(DEFAULT_OBJECT_GREEN),
      blue(DEFAULT_OBJECT_BLUE), object_category(Stationary), pos_x(DEFAULT_OBJECT_POSITION),
      pos_y(DEFAULT_OBJECT_POSITION), vel_x(DEFAULT_OBJECT_VELOCITY),
      vel_y(DEFAULT_OBJECT_VELOCITY), size_x(DEFAULT_OBJECT_SIZE), size_y(DEFAULT_OBJECT_SIZE),
      acc_x(DEFAULT_OBJECT_X_ACCELERATION), acc_y(GRAVITY_ACCELERATION), colliding(false),
      surface(nullptr), texture(nullptr), theta_x(0), callback([]() {}) {}

void GameObject::SetCallback(std::function<void()> callback) { this->callback = callback; }

void GameObject::Update() {}

void GameObject::MoveObject(float time) {
    if (this->object_category != Stationary and !colliding) {
        const float HALF = 0.5;
        pos_x += ((vel_x * time) + (HALF * acc_x * float(pow(time, 2))));
        pos_y += ((vel_y * time) + (HALF * acc_y * float(pow(time, 2))));

        vel_x += (acc_x * time);
        vel_y += (acc_y * time);
    }
}

std::pair<float, float> GameObject::GetPosition() {
    return std::make_pair(this->pos_x, this->pos_y);
}

std::pair<float, float> GameObject::GetVelocity() {
    return std::make_pair(this->vel_x, this->vel_y);
}

std::pair<float, float> GameObject::GetAcceleration() {
    return std::make_pair(this->acc_x, this->acc_y);
}

float GameObject::GetAngle() { return this->theta_x; }

bool GameObject::GetColliding() { return this->colliding; }

void GameObject::SetPosition(float pos_x, float pos_y) {
    this->pos_x = pos_x;
    this->pos_y = pos_y;
}

void GameObject::SetVelocity(float vel_x, float vel_y) {
    this->vel_x = vel_x;
    this->vel_y = vel_y;
}

void GameObject::SetAcceleration(float acc_x, float acc_y) {
    this->acc_x = acc_x;
    this->acc_y = acc_y;
}

void GameObject::SetColliding(bool colliding) { this->colliding = colliding; }

void GameObject::SetColor(int red, int green, int blue) {
    this->red = red;
    this->green = green;
    this->blue = blue;
}

void GameObject::SetShape(Shape shape) { this->shape = shape; }

void GameObject::SetAngle(float theta_x) { this->theta_x = theta_x; }