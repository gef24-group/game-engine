#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "SDL_render.h"
#include "Types.hpp"

class Render : public Component {
  private:
    Entity *entity;
    bool visible;
    std::string texture_path;
    SDL_Texture *texture;
    std::string texture_template;
    Shape shape;
    Color color;
    Border border;
    int depth;
    std::shared_ptr<Entity> camera;

  public:
    Render(Entity *entity);

    std::string GetTexturePath();
    SDL_Texture *GetTexture();
    std::string GetTextureTemplate();
    Shape GetShape();
    Color GetColor();
    Border GetBorder();
    int GetDepth();

    void SetCamera(std::shared_ptr<Entity> camera);
    void SetVisible(bool visible);
    void SetTexture(std::string path);
    void SetTextureTemplate(std::string texture_template);
    void SetShape(Shape shape);
    void SetColor(Color color);
    void SetBorder(Border border);
    void SetDepth(int depth);

    void RenderEntity();

    void Update() override;
};