#include "GameObject.hpp"
#include "SDL_render.h"
#include "Types.hpp"
#include "Utils.hpp"
#include <cmath>
#include <cstdint>
#include <functional>
#include <mutex>

GameObject::GameObject(std::string name, GameObjectCategory category) {
    this->name = name;
    this->category = category;
}

template <typename T, typename... Args> void GameObject::AddComponent(Args &&...args) {
    components[typeid(T)] = std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T> T *GameObject::GetComponent() {
    auto iterator = components.find(typeid(T));
    if (iterator != components.end()) {
        return dynamic_cast<T *>(iterator->second.get());
    }
    return nullptr;
}

void GameObject::Update(int64_t delta) {
    for (auto &[type, component] : components) {
        component->Update(this, delta); // Update each component, passing the current GameObject
    }
}

// GameObject::GameObject(std::string name, GameObjectCategory category) {
//     this->surface = nullptr;
//     this->texture = nullptr;
//     this->name = name;
//     this->category = category;
//     this->shape = Rectangle;
//     this->color = Color{0, 0, 0, 255};
//     this->border = Border{false, Color{0, 0, 0, 255}};
//     this->SetPosition(Position{0, 0});
//     this->size = Size{0, 0};
//     this->velocity = Velocity{0, 0};
//     this->acceleration = Acceleration{0, 0};
//     this->affected_by_collision = true;
//     this->restitution = 0;
//     this->theta_x = 0;
//     this->colliders = std::unordered_set<GameObject *>();
//     this->callback = [](GameObject *) {};
//     this->player_address = "";
//     this->owner = NetworkRole::Client;
// }

// void GameObject::Update() { this->callback(this); }

// void GameObject::Move(int64_t delta) {
//     const float HALF = 0.5;
//     float time = static_cast<float>(delta) / 100'000'000.0f;
//     Position curr_position = this->GetPosition();
//     float new_pos_x = curr_position.x + (this->velocity.x * time) +
//                       (HALF * this->acceleration.x * float(pow(time, 2)));
//     float new_pos_y = curr_position.y + (this->velocity.y * time) +
//                       (HALF * this->acceleration.y * float(pow(time, 2)));

//     this->SetPosition(Position{new_pos_x, new_pos_y});

//     this->velocity.x += (this->acceleration.x * time);
//     this->velocity.y += (this->acceleration.y * time);
// }

// void GameObject::Render() {
//     Position position = this->GetPosition();
//     float pos_x = position.x;
//     float pos_y = position.y;
//     int width = this->size.width;
//     int height = this->size.height;
//     int red = this->color.red;
//     int green = this->color.green;
//     int blue = this->color.blue;
//     int alpha = this->color.alpha;

//     SDL_Rect object = {static_cast<int>(std::round(pos_x)), static_cast<int>(std::round(pos_y)),
//                        width, height};
//     SDL_SetRenderDrawColor(app->renderer, red, green, blue, alpha);
//     if (this->shape == Rectangle && this->texture == nullptr) {
//         SDL_RenderFillRect(app->renderer, &object);
//     } else {
//         SDL_RenderCopy(app->renderer, this->texture, NULL, &object);
//         if (this->border.show) {
//             SDL_SetRenderDrawColor(app->renderer, this->border.color.red,
//             this->border.color.green,
//                                    this->border.color.blue, this->border.color.alpha);
//             SDL_RenderDrawRect(app->renderer, &object);
//         }
//     }
// }

// SDL_Texture *GameObject::GetTexture() { return this->texture; }
// std::string GameObject::GetTextureTemplate() { return this->texture_template; }
std::string GameObject::GetName() { return this->name; }
GameObjectCategory GameObject::GetCategory() { return this->category; }
// Shape GameObject::GetShape() { return this->shape; }
// Color GameObject::GetColor() { return this->color; }
// Border GameObject::GetBorder() { return this->border; }
// Position GameObject::GetPosition() {
//     std::lock_guard<std::mutex> lock(this->position_mutex);
//     return this->position;
// }
// Size GameObject::GetSize() { return this->size; }
// Velocity GameObject::GetVelocity() { return this->velocity; }
// Acceleration GameObject::GetAcceleration() { return this->acceleration; }
// bool GameObject::GetAffectedByCollision() { return this->affected_by_collision; }
// float GameObject::GetRestitution() { return this->restitution; }
// float GameObject::GetAngle() { return this->theta_x; }
// std::unordered_set<GameObject *> GameObject::GetColliders() { return this->colliders; }
// std::function<void(GameObject *)> GameObject::GetCallback() { return this->callback; }
// std::string GameObject::GetPlayerAddress() { return this->player_address; }
// NetworkRole GameObject::GetOwner() { return this->owner; }

// void GameObject::SetTexture(std::string path) { this->texture = LoadTexture(path); }
// void GameObject::SetTextureTemplate(std::string texture_template) {
//     this->texture_template = texture_template;
// }
void GameObject::SetName(std::string name) { this->name = name; }
// void GameObject::SetShape(Shape shape) { this->shape = shape; }
// void GameObject::SetColor(Color color) { this->color = color; }
// void GameObject::SetBorder(Border border) { this->border = border; }
// void GameObject::SetPosition(Position position) {
//     std::lock_guard<std::mutex> lock(this->position_mutex);
//     this->position = position;
// }
// void GameObject::SetSize(Size size) { this->size = size; }
// void GameObject::SetVelocity(Velocity velocity) {
//     if (this->category != Stationary) {
//         this->velocity = velocity;
//     }
// }
// void GameObject::SetAcceleration(Acceleration acceleration) {
//     if (this->category != Stationary) {
//         this->acceleration = acceleration;
//     }
// }
// void GameObject::SetAffectedByCollision(bool affected_by_collision) {
//     this->affected_by_collision = affected_by_collision;
// }
// void GameObject::SetRestitution(float restitution) { this->restitution = restitution; }
// void GameObject::SetAngle(float theta_x) { this->theta_x = theta_x; }
// void GameObject::AddCollider(GameObject *game_object) { this->colliders.insert(game_object); }
// void GameObject::RemoveCollider(GameObject *game_object) { this->colliders.erase(game_object); }
// void GameObject::SetCallback(std::function<void(GameObject *)> callback) {
//     this->callback = callback;
// }
// void GameObject::SetPlayerAddress(std::string player_address) {
//     this->player_address = player_address;
// }
// void GameObject::SetOwner(NetworkRole owner) { this->owner = owner; }