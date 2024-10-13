#include "Entity.hpp"
#include "Types.hpp"

Entity::Entity(std::string name, EntityCategory category) {
    this->name = name;
    this->category = category;
    this->active = true;
}

std::string Entity::GetName() { return this->name; }
EntityCategory Entity::GetCategory() { return this->category; }
bool Entity::GetActive() { return this->active.load(); }

void Entity::SetName(std::string name) { this->name = name; }
void Entity::SetActive(bool active) { this->active.store(active); }