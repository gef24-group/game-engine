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
    GameObjectCategory category;
    std::string uuid;
    std::string name;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;

  public:
    GameObject(std::string name, GameObjectCategory category);
    GameObjectCategory GetCategory();
    std::string GetName();

    void SetName(std::string name);

    template <typename T, typename... Args> void AddComponent(Args &&...args);
    template <typename T> T *GetComponent();

    void Update(int64_t delta);
};