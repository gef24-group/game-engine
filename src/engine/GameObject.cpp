#include "GameObject.hpp"
#include "Constants.hpp"
#include <cmath>

GameObject::GameObject(GameObjectCategory object_category)
    : shape(Rectangle), red(0), green(DEFAULT_OBJECT_GREEN), blue(DEFAULT_OBJECT_BLUE),
      object_category(Stationary), pos_x(DEFAULT_OBJECT_POSITION), pos_y(DEFAULT_OBJECT_POSITION),
      vel_x(DEFAULT_OBJECT_VELOCITY), vel_y(DEFAULT_OBJECT_VELOCITY), size_x(DEFAULT_OBJECT_SIZE),
      size_y(DEFAULT_OBJECT_SIZE), acc_x(DEFAULT_OBJECT_X_ACCELERATION),
      acc_y(GRAVITY_ACCELERATION), colliding(false), surface(nullptr), texture(nullptr),
      theta_x(-1) {}

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

// void GameObject::UpdateMotionProperties(float pos_x = 0, float pos_y = 0, float vel_x = 0,
//                                         float vel_y = 0, float acc_x = 0,
//                                         float acc_y = GRAVITY_ACCELERATION, bool colliding =
//                                         false, bool gravity_applies = false) {
//     this->pos_x = pos_x;
//     this->pos_y = pos_y;
//     this->vel_x = vel_x;
//     this->vel_y = vel_y;
//     this->acc_x = acc_x;
//     this->acc_y = acc_y;
//     this->colliding = colliding;
//     this->gravity_applies = gravity_applies;
// }

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

// void GameObject::SetGravityApplies(bool gravity_applies) {
//     this->gravity_applies = gravity_applies;
// }

void GameObject::SetColor(int red, int green, int blue) {
    this->red = red;
    this->green = green;
    this->blue = blue;
}

void GameObject::SetShape(Shape shape) { this->shape = shape; }

void GameObject::SetAngle(float theta_x) { this->theta_x = theta_x; }