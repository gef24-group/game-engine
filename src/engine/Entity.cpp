#include "Entity.hpp"
#include "Types.hpp"

Entity::Entity(std::string name, EntityCategory category) {
    this->name = name;
    this->category = category;
}

std::string Entity::GetName() { return this->name; }
EntityCategory Entity::GetCategory() { return this->category; }

void Entity::SetName(std::string name) { this->name = name; }