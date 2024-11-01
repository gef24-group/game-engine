#include "App.hpp"
#include "EventHandler.hpp"
#include "SDL_scancode.h"
#include "SDL_stdinc.h"

extern App *app;

class Input : public EventHandler {
  private:
    Uint8 *keyboard_state;
    std::unordered_map<SDL_Scancode, Key *> sdl_scancode_to_key_map;

    void RaiseEvent(SDL_Scancode key, bool is_pressed);

  public:
    Input();
    void CheckKeyboardState();
    void OnEvent(Event event) override;
};