#include "Transform.hpp"
#include "GameObject.hpp"

Transform::Transform(GameObject *game_object) {
    this->game_object = game_object;
    this->SetPosition(Position{0, 0});
    this->size = Size{0, 0};
}

Position Transform::GetPosition() {
    std::lock_guard<std::mutex> lock(this->position_mutex);
    return this->position;
}
Size Transform::GetSize() { return this->size; }

void Transform::SetPosition(Position position) {
    std::lock_guard<std::mutex> lock(this->position_mutex);
    this->position = position;
}
void Transform::SetSize(Size size) { this->size = size; }

void Transform::Update() {};