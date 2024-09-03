#include "GameObject.hpp"
#include "SDL_render.h"
#include "Types.hpp"
#include "Utils.hpp"
#include <cmath>
#include <functional>

GameObject::GameObject(std::string name, GameObjectCategory category) {
    this->surface = nullptr;
    this->texture = nullptr;
    this->name = name;
    this->category = category;
    this->shape = Rectangle;
    this->color = Color{0, 0, 0, 255};
    this->position = Position{0, 0};
    this->size = Size{0, 0};
    this->velocity = Velocity{0, 0};
    this->acceleration = Acceleration{0, 0};
    this->restitution = 0;
    this->theta_x = 0;
    this->colliders = std::unordered_set<GameObject *>();
    this->callback = [](GameObject *) {};
}

void GameObject::Update() { this->callback(this); }

void GameObject::Move(float time) {
    const float HALF = 0.5;
    this->position.x +=
        (this->velocity.x * time) + (HALF * this->acceleration.x * float(pow(time, 2)));
    this->position.y +=
        (this->velocity.y * time) + (HALF * this->acceleration.y * float(pow(time, 2)));

    this->velocity.x += (this->acceleration.x * time);
    this->velocity.y += (this->acceleration.y * time);
}

void GameObject::Render() {
    float pos_x = this->position.x;
    float pos_y = this->position.y;
    int width = this->size.width;
    int height = this->size.height;
    int red = this->color.red;
    int green = this->color.green;
    int blue = this->color.blue;
    int alpha = this->color.alpha;

    SDL_Rect object = {static_cast<int>(std::round(pos_x)), static_cast<int>(std::round(pos_y)),
                       width, height};
    SDL_SetRenderDrawColor(app->renderer, red, green, blue, alpha);
    if (this->shape == Rectangle && this->texture == nullptr) {
        SDL_RenderFillRect(app->renderer, &object);
    } else {
        SDL_RenderCopy(app->renderer, this->texture, NULL, &object);
    }
}

SDL_Texture *GameObject::GetTexture() { return this->texture; }
std::string GameObject::GetName() { return this->name; }
GameObjectCategory GameObject::GetCategory() { return this->category; }
Shape GameObject::GetShape() { return this->shape; }
Color GameObject::GetColor() { return this->color; }
Position GameObject::GetPosition() { return this->position; }
Size GameObject::GetSize() { return this->size; }
Velocity GameObject::GetVelocity() { return this->velocity; }
Acceleration GameObject::GetAcceleration() { return this->acceleration; }
float GameObject::GetRestitution() { return this->restitution; }
float GameObject::GetAngle() { return this->theta_x; }
std::unordered_set<GameObject *> GameObject::GetColliders() { return this->colliders; }

void GameObject::SetTexture(std::string path) { this->texture = LoadTexture(path); }
void GameObject::SetShape(Shape shape) { this->shape = shape; }
void GameObject::SetColor(Color color) { this->color = color; }
void GameObject::SetPosition(Position position) { this->position = position; }
void GameObject::SetSize(Size size) { this->size = size; }
void GameObject::SetVelocity(Velocity velocity) {
    if (this->category != Stationary) {
        this->velocity = velocity;
    }
}
void GameObject::SetAcceleration(Acceleration acceleration) {
    if (this->category != Stationary) {
        this->acceleration = acceleration;
    }
}
void GameObject::SetRestitution(float restitution) { this->restitution = restitution; }
void GameObject::SetAngle(float theta_x) { this->theta_x = theta_x; }
void GameObject::AddCollider(GameObject *game_object) { this->colliders.insert(game_object); }
void GameObject::RemoveCollider(GameObject *game_object) { this->colliders.erase(game_object); }
void GameObject::SetCallback(std::function<void(GameObject *)> callback) {
    this->callback = callback;
}