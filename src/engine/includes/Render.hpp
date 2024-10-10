#pragma once

#include "Component.hpp"
#include "GameObject.hpp"
#include "SDL_render.h"
#include "Types.hpp"

class Render : public Component {
  private:
    GameObject *game_object;
    SDL_Texture *texture;
    std::string texture_template;
    Shape shape;
    Color color;
    Border border;

  public:
    Render(GameObject *game_object);

    SDL_Texture *GetTexture();
    std::string GetTextureTemplate();
    Shape GetShape();
    Color GetColor();
    Border GetBorder();

    void SetTexture(std::string path);
    void SetTextureTemplate(std::string texture_template);
    void SetShape(Shape shape);
    void SetColor(Color color);
    void SetBorder(Border border);

    void RenderObject();

    void Update() override;
};