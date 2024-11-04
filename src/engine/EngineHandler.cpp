#include "EngineHandler.hpp"
#include "Engine.hpp"
#include "EventManager.hpp"
#include "SDL_scancode.h"

EngineHandler::EngineHandler() {
    EventManager::GetInstance().Register({EventType::Input, EventType::Join, EventType::Leave},
                                         this);

    this->pause_key = SDL_SCANCODE_P;
    this->speed_down_key = SDL_SCANCODE_COMMA;
    this->speed_up_key = SDL_SCANCODE_PERIOD;
    this->display_scaling_key = SDL_SCANCODE_X;
    this->hidden_zone_key = SDL_SCANCODE_Z;
}

void EngineHandler::BindPauseKey(SDL_Scancode key) { this->pause_key = key; }
void EngineHandler::BindSpeedDownKey(SDL_Scancode key) { this->speed_down_key = key; }
void EngineHandler::BindSpeedUpKey(SDL_Scancode key) { this->speed_up_key = key; }
void EngineHandler::BindDisplayScalingKey(SDL_Scancode key) { this->display_scaling_key = key; }
void EngineHandler::BindHiddenZoneKey(SDL_Scancode key) { this->hidden_zone_key = key; }

void EngineHandler::OnEvent(Event event) {
    if (event.type != EventType::Input) {
        return;
    }
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));
    if (input_event == nullptr) {
        return;
    }
    bool pressed = input_event->pressed;
    if (!pressed) {
        return;
    }

    SDL_Scancode key = input_event->keys[0];
    if (key == this->pause_key) {
        Engine::GetInstance().EngineTimelineTogglePause();
    }
    if (key == this->speed_down_key) {
        Engine::GetInstance().EngineTimelineChangeTic(
            std::min(Engine::GetInstance().EngineTimelineGetTic() * 2.0, 2.0));
    }
    if (key == this->speed_up_key) {
        Engine::GetInstance().EngineTimelineChangeTic(
            std::max(Engine::GetInstance().EngineTimelineGetTic() / 2.0, 0.5));
    }
    if (key == this->display_scaling_key) {
        app->window.proportional_scaling = !app->window.proportional_scaling;
    }
    if (key == this->hidden_zone_key) {
        Engine::GetInstance().ToggleShowZoneBorders();
    }
}