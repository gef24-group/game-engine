#include "Render.hpp"
#include "GameObject.hpp"
#include "Transform.hpp"
#include "Utils.hpp"

Render::Render(GameObject *game_object) {
    this->game_object = game_object;
    this->texture = nullptr;
    this->texture_template = "";
    this->shape = Rectangle;
    this->color = Color{0, 0, 0, 255};
    this->border = Border{false, Color{0, 0, 0, 255}};
}

SDL_Texture *Render::GetTexture() { return this->texture; }
std::string Render::GetTextureTemplate() { return this->texture_template; }
Shape Render::GetShape() { return this->shape; }
Color Render::GetColor() { return this->color; }
Border Render::GetBorder() { return this->border; }

void Render::SetTexture(std::string path) { this->texture = LoadTexture(path); }
void Render::SetTextureTemplate(std::string texture_template) {
    this->texture_template = texture_template;
}
void Render::SetShape(Shape shape) { this->shape = shape; }
void Render::SetColor(Color color) { this->color = color; }
void Render::SetBorder(Border border) { this->border = border; }

void Render::RenderObject() {
    Position position = this->game_object->GetComponent<Transform>()->GetPosition();
    float pos_x = position.x;
    float pos_y = position.y;
    int width = this->game_object->GetComponent<Transform>()->GetSize().width;
    int height = this->game_object->GetComponent<Transform>()->GetSize().height;
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

void Render::Update() { this->RenderObject(); }