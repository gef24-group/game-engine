#include "Render.hpp"
#include "Entity.hpp"
#include "Transform.hpp"
#include "Utils.hpp"

Render::Render(Entity *entity) {
    this->entity = entity;
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

void Render::RenderEntity() {
    Position position = this->entity->GetComponent<Transform>()->GetPosition();
    int pos_x = static_cast<int>(std::round(position.x));
    int pos_y = static_cast<int>(std::round(position.y));
    int width = this->entity->GetComponent<Transform>()->GetSize().width;
    int height = this->entity->GetComponent<Transform>()->GetSize().height;
    int red = this->color.red;
    int green = this->color.green;
    int blue = this->color.blue;
    int alpha = this->color.alpha;

    SDL_Rect rectangle = {pos_x, pos_y, width, height};
    SDL_SetRenderDrawColor(app->renderer, red, green, blue, alpha);
    if (this->shape == Rectangle && this->texture == nullptr) {
        SDL_RenderFillRect(app->renderer, &rectangle);
    } else {
        SDL_RenderCopy(app->renderer, this->texture, NULL, &rectangle);
        if (this->border.show) {
            SDL_SetRenderDrawColor(app->renderer, this->border.color.red, this->border.color.green,
                                   this->border.color.blue, this->border.color.alpha);
            SDL_RenderDrawRect(app->renderer, &rectangle);
        }
    }
}

void Render::Update() { this->RenderEntity(); }