#include "Render.hpp"
#include "Entity.hpp"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"

Render::Render(Entity *entity) {
    this->entity = entity;
    this->texture = nullptr;
    this->texture_template = "";
    this->shape = Rectangle;
    this->color = Color{0, 0, 0, 255};
    this->border = Border{false, Color{0, 0, 0, 255}};
    this->camera = std::make_shared<Entity>("camera", EntityCategory::Camera);
}

SDL_Texture *Render::GetTexture() { return this->texture; }
std::string Render::GetTextureTemplate() { return this->texture_template; }
Shape Render::GetShape() { return this->shape; }
Color Render::GetColor() { return this->color; }
Border Render::GetBorder() { return this->border; }
Position Render::GetScreenPosition() {
    float screen_pos_x, screen_pos_y;
    Position world_position = this->entity->GetComponent<Transform>()->GetPosition();
    Position camera_position = this->camera->GetComponent<Transform>()->GetPosition();

    screen_pos_x = world_position.x - camera_position.x;
    screen_pos_y = world_position.y - camera_position.y;

    if (this->entity->GetName() == "side_boundary_1") {
        Log(LogLevel::Info,
            "Camera pos x: %f , camera pos y: %f, screen pos x: %f, screen pos y: %f, world pos x: "
            "%f, world pos y: %f",
            camera_position.x, camera_position.y, screen_pos_x, screen_pos_y, world_position.x,
            world_position.y);
    }

    return Position{screen_pos_x, screen_pos_y};
}

void Render::SetCamera(std::shared_ptr<Entity> camera) { this->camera = camera; }

void Render::SetTexture(std::string path) { this->texture = LoadTexture(path); }
void Render::SetTextureTemplate(std::string texture_template) {
    this->texture_template = texture_template;
}
void Render::SetShape(Shape shape) { this->shape = shape; }
void Render::SetColor(Color color) { this->color = color; }
void Render::SetBorder(Border border) { this->border = border; }

void Render::RenderEntity() {
    Position position = this->GetScreenPosition();
    int pos_x = static_cast<int>(std::round(position.x));
    int pos_y = static_cast<int>(std::round(position.y));
    int width = this->entity->GetComponent<Transform>()->GetSize().width;
    int height = this->entity->GetComponent<Transform>()->GetSize().height;
    int fill_r = this->color.red;
    int fill_g = this->color.green;
    int fill_b = this->color.blue;
    int fill_a = this->color.alpha;
    int border_r = this->border.color.red;
    int border_g = this->border.color.green;
    int border_b = this->border.color.blue;
    int border_a = this->border.color.alpha;

    SDL_Rect rectangle = {pos_x, pos_y, width, height};
    if (this->shape == Rectangle && this->texture == nullptr) {
        if (this->border.show) {
            SDL_SetRenderDrawColor(app->renderer, border_r, border_g, border_b, border_a);
            SDL_RenderDrawRect(app->renderer, &rectangle);
        } else {
            SDL_SetRenderDrawColor(app->renderer, fill_r, fill_g, fill_b, fill_a);
            SDL_RenderFillRect(app->renderer, &rectangle);
        }
    } else {
        SDL_RenderCopy(app->renderer, this->texture, NULL, &rectangle);
        if (this->border.show) {
            SDL_SetRenderDrawColor(app->renderer, border_r, border_g, border_b, border_a);
            SDL_RenderDrawRect(app->renderer, &rectangle);
        }
    }
}

void Render::Update() { this->RenderEntity(); }