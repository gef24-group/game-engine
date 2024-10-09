#include "RenderComponent.hpp"
#include "GameObject.hpp"
#include "PhysicsComponent.hpp"
#include "Utils.hpp"
#include <cstdint>

RenderComponent::RenderComponent() {
    this->surface = nullptr;
    this->texture = nullptr;
    this->shape = Rectangle;
    this->color = Color{0, 0, 0, 255};
    this->border = Border{false, Color{0, 0, 0, 255}};
}

SDL_Texture *RenderComponent::GetTexture() { return this->texture; }
std::string RenderComponent::GetTextureTemplate() { return this->texture_template; }
Shape RenderComponent::GetShape() { return this->shape; }
Color RenderComponent::GetColor() { return this->color; }
Border RenderComponent::GetBorder() { return this->border; }

void RenderComponent::SetTexture(std::string path) { this->texture = LoadTexture(path); }
void RenderComponent::SetTextureTemplate(std::string texture_template) {
    this->texture_template = texture_template;
}
void RenderComponent::SetShape(Shape shape) { this->shape = shape; }
void RenderComponent::SetColor(Color color) { this->color = color; }
void RenderComponent::SetBorder(Border border) { this->border = border; }

void RenderComponent::Render(GameObject *game_object) {
    Position position = game_object->GetComponent<PhysicsComponent>()->GetPosition();
    float pos_x = position.x;
    float pos_y = position.y;
    int width = game_object->GetComponent<PhysicsComponent>()->GetSize().width;
    int height = game_object->GetComponent<PhysicsComponent>()->GetSize().height;
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
        if (this->border.show) {
            SDL_SetRenderDrawColor(app->renderer, this->border.color.red, this->border.color.green,
                                   this->border.color.blue, this->border.color.alpha);
            SDL_RenderDrawRect(app->renderer, &object);
        }
    }
}

void RenderComponent::Update(GameObject *game_object, int64_t delta) { Render(game_object); }