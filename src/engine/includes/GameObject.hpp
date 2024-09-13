#pragma once

#include "App.hpp"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "Types.hpp"
#include <functional>
#include <string>
#include <unordered_set>

extern App *app;

class GameObject {
  private:
    SDL_Surface *surface;
    SDL_Texture *texture;
    std::string name;
    GameObjectCategory category;
    Shape shape;
    Color color;
    Position position;
    Size size;
    Velocity velocity;
    Acceleration acceleration;
    bool reduce_velocity_on_collision;
    float restitution;
    // Angle the object makes with the x_axis
    float theta_x;

    std::unordered_set<GameObject *> colliders;
    std::function<void(GameObject *)> callback;

  public:
    GameObject(std::string name, GameObjectCategory category);
    void Update();
    void Move(int64_t delta);
    void Render();

    SDL_Texture *GetTexture();
    std::string GetName();
    GameObjectCategory GetCategory();
    Shape GetShape();
    Color GetColor();
    Position GetPosition();
    Size GetSize();
    Velocity GetVelocity();
    Acceleration GetAcceleration();
    float GetRestitution();
    bool GetReduceVelocityOnCollision();
    float GetAngle();
    std::unordered_set<GameObject *> GetColliders();

    void SetTexture(std::string path);
    void SetShape(Shape shape);
    void SetColor(Color color);
    void SetPosition(Position position);
    void SetSize(Size size);
    void SetVelocity(Velocity velocity);
    void SetAcceleration(Acceleration acceleration);
    void SetRestitution(float restitution);
    void SetReduceVelocityOnCollision(bool reduce_velocity_on_collision);
    void SetAngle(float theta_x);
    void AddCollider(GameObject *game_object);
    void RemoveCollider(GameObject *game_object);
    void SetCallback(std::function<void(GameObject *)> callback);
};