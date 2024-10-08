#include "GameObject.hpp"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "Types.hpp"

class RenderComponent : Component {
  private:
    SDL_Surface *surface;
    SDL_Texture *texture;
    std::string texture_template;
    std::string name;
    Shape shape;
    Color color;
    Border border;

  public:
    SDL_Texture *GetTexture();
    std::string GetTextureTemplate();
    std::string GetName();
    Shape GetShape();
    Color GetColor();
    Border GetBorder();

    void SetTexture(std::string path);
    void SetTextureTemplate(std::string texture_template);
    void SetName(std::string name);
    void SetShape(Shape shape);
    void SetColor(Color color);
    void SetBorder(Border border);

    void Render(GameObject *game_object);

    void Update(GameObject *game_object);
};