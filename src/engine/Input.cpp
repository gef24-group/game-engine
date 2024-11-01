#include "Input.hpp"
#include "EventHandler.hpp"
#include "EventManager.hpp"
#include "SDL_keyboard.h"
#include "SDL_scancode.h"
#include "Types.hpp"
#include "Utils.hpp"

Input::Input() {
    this->keyboard_state = nullptr;
    this->sdl_scancode_to_key_map = {{SDL_SCANCODE_W, &(app->key_map->key_W)},
                                     {SDL_SCANCODE_A, &(app->key_map->key_A)},
                                     {SDL_SCANCODE_S, &(app->key_map->key_S)},
                                     {SDL_SCANCODE_D, &(app->key_map->key_D)},
                                     {SDL_SCANCODE_X, &(app->key_map->key_X)},
                                     {SDL_SCANCODE_P, &(app->key_map->key_P)},
                                     {SDL_SCANCODE_Z, &(app->key_map->key_Z)},
                                     {SDL_SCANCODE_UP, &(app->key_map->key_up)},
                                     {SDL_SCANCODE_LEFT, &(app->key_map->key_left)},
                                     {SDL_SCANCODE_DOWN, &(app->key_map->key_down)},
                                     {SDL_SCANCODE_RIGHT, &(app->key_map->key_right)},
                                     {SDL_SCANCODE_SPACE, &(app->key_map->key_space)},
                                     {SDL_SCANCODE_COMMA, &(app->key_map->key_comma)},
                                     {SDL_SCANCODE_PERIOD, &(app->key_map->key_period)}};
}

// Event handler function to be called on key state change
void Input::RaiseEvent(SDL_Scancode key, bool is_pressed) {
    InputEvent input_event_data = InputEvent{{key}, InputEventType::Single, is_pressed};
    Event input_event = Event(EventType::Input, input_event_data);
    input_event.SetDelay(0);
    input_event.SetPriority(Priority::High);
    EventManager::GetInstance().Raise(input_event);
}

void Input::CheckKeyboardState() {
    const Uint8 *new_state = SDL_GetKeyboardState(NULL);

    // If first call, initialize prevState to match the current state
    if (!this->keyboard_state) {
        int num_keys;
        this->keyboard_state = new Uint8[SDL_NUM_SCANCODES];
        memcpy((void *)this->keyboard_state, new_state, SDL_NUM_SCANCODES);
    }

    // Iterate over all scancodes and check for changes
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
        if (new_state[i] != this->keyboard_state[i]) {
            bool is_pressed = new_state[i];
            this->RaiseEvent(static_cast<SDL_Scancode>(i), is_pressed);
        }
    }

    // Update the previous state
    memcpy((void *)this->keyboard_state, new_state, SDL_NUM_SCANCODES);
}

void Input::OnEvent(Event event) {
    EventType event_type = event.type;
    switch (event_type) {
    case EventType::Input: {
        InputEvent *input_event = std::get_if<InputEvent>(&(event.data));
        if (input_event) {
            for (int i = 0; i < 10; i++) {
                int key_scancode = input_event->keys[i];
                if (key_scancode == 0) {
                    // 0 is SDL_SCANCODE_UNKNOWN, not assigned to any key
                    // 0 also denotes the end of the list of keys pressed in the array
                    break;
                }
                // If the scancode is found in the map
                if (this->sdl_scancode_to_key_map.find(static_cast<SDL_Scancode>(key_scancode)) !=
                    this->sdl_scancode_to_key_map.end()) {
                    this->sdl_scancode_to_key_map[static_cast<SDL_Scancode>(key_scancode)]
                        ->pressed.store(input_event->pressed);
                }
            }
        } else {
            Log(LogLevel::Error, "Input event data not found in an event of type \'Input\'");
        }
        break;
    }
    default:
        break;
    }
}