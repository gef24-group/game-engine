#include "Render.hpp"
#include "Entity.hpp"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <SDL_rect.h>

Render::Render(Entity *entity) {
    this->entity = entity;
    this->visible = true;
    this->texture_path = "";
    this->texture = nullptr;
    this->texture_template = "";
    this->shape = Shape::Rectangle;
    this->color = Color{0, 0, 0, 255};
    this->border = Border{false, Color{0, 0, 0, 255}};
    this->depth = 0;
    this->camera = std::make_shared<Entity>("camera", EntityCategory::Camera);
}

std::string Render::GetTexturePath() { return this->texture_path; }
SDL_Texture *Render::GetTexture() { return this->texture; }
std::string Render::GetTextureTemplate() { return this->texture_template; }
Shape Render::GetShape() { return this->shape; }
Color Render::GetColor() { return this->color; }
Border Render::GetBorder() { return this->border; }
int Render::GetDepth() { return this->depth; }

void Render::SetCamera(std::shared_ptr<Entity> camera) { this->camera = camera; }

void Render::SetVisible(bool visible) { this->visible = visible; }
void Render::SetTexture(std::string path) {
    this->texture_path = path;
    this->texture = LoadTexture(path);
}
void Render::SetTextureTemplate(std::string texture_template) {
    this->texture_template = texture_template;
}
void Render::SetShape(Shape shape) { this->shape = shape; }
void Render::SetColor(Color color) { this->color = color; }
void Render::SetBorder(Border border) { this->border = border; }
void Render::SetDepth(int depth) { this->depth = depth; }

void Render::RenderEntity() {
    if (!this->visible) {
        return;
    }

    Position position = GetScreenPosition(this->entity->GetComponent<Transform>()->GetPosition(),
                                          this->camera->GetComponent<Transform>()->GetPosition());

    int pos_x = static_cast<int>(std::round(position.x));
    int pos_y = static_cast<int>(std::round(position.y));
    int width = this->entity->GetComponent<Transform>()->GetSize().width;
    int height = this->entity->GetComponent<Transform>()->GetSize().height;
    double angle = this->entity->GetComponent<Transform>()->GetAngle();
    SDL_Point anchor = this->entity->GetComponent<Transform>()->GetAnchor();
    SDL_Point *anchor_ptr = (anchor.x == 0 && anchor.y == 0) ? nullptr : &anchor;
    int fill_r = this->color.red;
    int fill_g = this->color.green;
    int fill_b = this->color.blue;
    int fill_a = this->color.alpha;
    int border_r = this->border.color.red;
    int border_g = this->border.color.green;
    int border_b = this->border.color.blue;
    int border_a = this->border.color.alpha;

    bool render_with_angle = false;
    int window_w, window_h;
    SDL_GetWindowSize(app->sdl_window, &window_w, &window_h);
    int logical_w = app->window.width;
    int logical_h = app->window.height;
    float window_aspect_ratio = float(window_w) / float(window_h);
    float logical_aspect_ratio = float(logical_w) / float(logical_h);
    if (std::fabs(logical_aspect_ratio - window_aspect_ratio) <= 0.2f) {
        render_with_angle = true;
    }

    SDL_Rect rectangle = {pos_x, pos_y, width, height};
    if (this->shape == Shape::Rectangle && this->texture == nullptr) {
        if (this->border.show) {
            SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(app->renderer, border_r, border_g, border_b, border_a);
            SDL_RenderDrawRect(app->renderer, &rectangle);
        } else {
            SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(app->renderer, fill_r, fill_g, fill_b, fill_a);
            SDL_RenderFillRect(app->renderer, &rectangle);
        }
    } else {
        if (render_with_angle) {
            SDL_RenderCopyEx(app->renderer, this->texture, NULL, &rectangle, angle, anchor_ptr,
                             SDL_FLIP_NONE);
        } else {
            SDL_RenderCopy(app->renderer, this->texture, NULL, &rectangle);
        }

        if (this->border.show) {
            SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(app->renderer, border_r, border_g, border_b, border_a);
            SDL_RenderDrawRect(app->renderer, &rectangle);
        }
    }
}

void Render::Update() { this->RenderEntity(); }