#pragma once
#include "SDL_render.h"
#include "SDL_surface.h"
#include "Types.hpp"
#include <utility>

class GameObject {
  private:
    SDL_Surface *surface;
    SDL_Texture *texture;
    // Horizontal and vertical lengths of the object wrt its top-left pixel
    Shape shape;
    // Color intensities
    int red, green, blue;
    GameObjectCategory object_category;
    float size_x, size_y;
    float pos_x, pos_y;
    float vel_x, vel_y;
    float acc_x, acc_y;
    // Angle the object makes with the x_axis
    float theta_x;
    bool colliding;
    // bool gravity_applies;

  public:
    GameObject(GameObjectCategory object_category);
    void Update();
    void MoveObject(float time);
    // void UpdateMotionProperties(float pos_x, float pos_y, float vel_x, float vel_y, float acc_x,
    //                             float acc_y, bool colliding, bool gravity_applies);

    std::pair<float, float> GetPosition();
    std::pair<float, float> GetVelocity();
    std::pair<float, float> GetAcceleration();
    float GetAngle();
    bool GetColliding();
    bool GetGravityApplies();

    void SetPosition(float pos_x, float pos_y);
    void SetVelocity(float vel_x, float vel_y);
    void SetAcceleration(float acc_x, float acc_y);
    void SetColliding(bool colliding);
    // void SetGravityApplies(bool gravity_applies);

    void SetColor(int red, int green, int blue);
    void SetShape(Shape shape);
    void SetAngle(float theta_x);
    bool SetTexture();
};