#include "Input.hpp"
#include "EventManager.hpp"
#include "SDL_keyboard.h"
#include "SDL_scancode.h"
#include "Types.hpp"

Input::Input() { this->keyboard_state = nullptr; }

// Event handler function to be called on key state change
void Input::RaiseEvent(SDL_Scancode key, bool is_pressed) {
    EventManager::GetInstance().RaiseInputEvent(
        InputEvent{{key}, InputEventType::Single, is_pressed});
}

void Input::Process() {
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