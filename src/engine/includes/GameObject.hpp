#pragma once

#include "App.hpp"
#include "Component.hpp"
#include "Types.hpp"
#include <memory>
#include <string>
#include <typeindex>

extern App *app;

class GameObject {
  private:
    std::string name;
    GameObjectCategory category;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;

  public:
    GameObject(std::string name, GameObjectCategory category);

    std::string GetName();
    GameObjectCategory GetCategory();

    void SetName(std::string name);

    template <typename T> void AddComponent();
    template <typename T> T *GetComponent();
};

template <typename T> void GameObject::AddComponent() {
    components[typeid(T)] = std::make_unique<T>(this);
}

template <typename T> T *GameObject::GetComponent() {
    auto iterator = components.find(typeid(T));
    if (iterator != components.end()) {
        return dynamic_cast<T *>(iterator->second.get());
    }
    return nullptr;
}