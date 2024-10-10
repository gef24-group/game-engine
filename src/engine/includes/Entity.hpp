#pragma once

#include "App.hpp"
#include "Component.hpp"
#include "Types.hpp"
#include <memory>
#include <string>
#include <typeindex>

extern App *app;

class Entity {
  private:
    std::string name;
    EntityCategory category;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;

  public:
    Entity(std::string name, EntityCategory category);

    std::string GetName();
    EntityCategory GetCategory();

    void SetName(std::string name);

    template <typename T> void AddComponent();
    template <typename T> T *GetComponent();
};

template <typename T> void Entity::AddComponent() {
    components[typeid(T)] = std::make_unique<T>(this);
}

template <typename T> T *Entity::GetComponent() {
    auto iterator = components.find(typeid(T));
    if (iterator != components.end()) {
        return dynamic_cast<T *>(iterator->second.get());
    }
    return nullptr;
}