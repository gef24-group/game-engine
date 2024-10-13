#pragma once

#include "App.hpp"
#include "Component.hpp"
#include "Types.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>

extern App *app;

class Entity {
  private:
    std::string name;
    EntityCategory category;
    std::atomic<bool> active;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
    std::mutex components_mutex;

  public:
    Entity(std::string name, EntityCategory category);

    std::string GetName();
    EntityCategory GetCategory();
    bool GetActive();

    void SetName(std::string name);
    void SetActive(bool active);

    template <typename T> void AddComponent();
    template <typename T> T *GetComponent();
    template <typename T> void RemoveComponent();
};

template <typename T> void Entity::AddComponent() {
    std::lock_guard<std::mutex> lock(components_mutex);
    components[typeid(T)] = std::make_unique<T>(this);
}

template <typename T> T *Entity::GetComponent() {
    std::lock_guard<std::mutex> lock(components_mutex);
    auto iterator = components.find(typeid(T));
    if (iterator != components.end()) {
        return dynamic_cast<T *>(iterator->second.get());
    }
    return nullptr;
}

template <typename T> void Entity::RemoveComponent() {
    std::lock_guard<std::mutex> lock(components_mutex);
    components.erase(typeid(T));
}