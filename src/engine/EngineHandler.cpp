#include "EngineHandler.hpp"
#include "Engine.hpp"
#include "EventManager.hpp"
#include "SDL_scancode.h"

EngineHandler::EngineHandler() {
    this->pause_key = SDL_SCANCODE_P;
    this->speed_down_key = SDL_SCANCODE_COMMA;
    this->speed_up_key = SDL_SCANCODE_PERIOD;
    this->display_scaling_key = SDL_SCANCODE_X;
    this->hidden_zone_key = SDL_SCANCODE_Z;

    EventManager::GetInstance().Register(
        {EventType::Input, EventType::Join, EventType::Discover, EventType::Leave}, this);
}

void EngineHandler::BindPauseKey(SDL_Scancode key) { this->pause_key = key; }
void EngineHandler::BindSpeedDownKey(SDL_Scancode key) { this->speed_down_key = key; }
void EngineHandler::BindSpeedUpKey(SDL_Scancode key) { this->speed_up_key = key; }
void EngineHandler::BindDisplayScalingKey(SDL_Scancode key) { this->display_scaling_key = key; }
void EngineHandler::BindHiddenZoneKey(SDL_Scancode key) { this->hidden_zone_key = key; }

void EngineHandler::HandleEngineInput(Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));
    if (input_event == nullptr) {
        return;
    }
    bool pressed = input_event->pressed;
    if (!pressed) {
        return;
    }

    SDL_Scancode key = input_event->key;
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

void EngineHandler::OnEvent(Event event) {
    EventType event_type = event.type;

    switch (event_type) {
    case EventType::Input:
        this->HandleEngineInput(event);
        break;
    case EventType::Join: {
        JoinEvent *join_event = std::get_if<JoinEvent>(&(event.data));
        Engine::GetInstance().OnJoin(join_event->player_address);

        break;
    }
    case EventType::Discover:
        Engine::GetInstance().OnDiscover();
        break;
    case EventType::Leave:
        Engine::GetInstance().OnLeave();
        break;
    default:
        break;
    }
}