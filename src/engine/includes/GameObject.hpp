#pragma once

#include "App.hpp"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "Types.hpp"
#include <functional>
#include <string>

extern App *app;

class GameObject {
  private:
    SDL_Surface *surface;
    SDL_Texture *texture;
    GameObjectCategory category;
    Shape shape;
    Color color;
    Position position;
    Size size;
    Velocity velocity;
    Acceleration acceleration;
    // Angle the object makes with the x_axis
    float theta_x;
    bool collided;
    std::function<void(GameObject *)> callback;

  public:
    GameObject(GameObjectCategory category);
    void Update();
    void Move(float time);

    SDL_Texture *GetTexture();
    Shape GetShape();
    Color GetColor();
    Position GetPosition();
    Size GetSize();
    Velocity GetVelocity();
    Acceleration GetAcceleration();
    float GetAngle();
    bool GetCollided();

    void SetTexture(std::string path);
    void SetShape(Shape shape);
    void SetColor(Color color);
    void SetPosition(Position position);
    void SetSize(Size size);
    void SetVelocity(Velocity velocity);
    void SetAcceleration(Acceleration acceleration);
    void SetAngle(float theta_x);
    void SetCollided(bool collided);
    void SetCallback(std::function<void(GameObject *)> callback);
};