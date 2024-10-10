#include "GameObject.hpp"
#include "Types.hpp"

GameObject::GameObject(std::string name, GameObjectCategory category) {
    this->name = name;
    this->category = category;
}

std::string GameObject::GetName() { return this->name; }
GameObjectCategory GameObject::GetCategory() { return this->category; }

void GameObject::SetName(std::string name) { this->name = name; }