#include "Input.hpp"
#include "Chord.hpp"
#include "EventManager.hpp"
#include "SDL_keyboard.h"
#include "SDL_scancode.h"
#include "Types.hpp"
#include <cstddef>

Input::Input() {
    this->keyboard_state = nullptr;
    this->chords = std::vector<Chord>();
}

bool Input::IsKeyInChords(SDL_Scancode key) {
    for (Chord &chord : this->chords) {
        if (chord.IsKeyInChord(key)) {
            return true;
        }
    }
    return false;
}

void Input::Process() {
    const Uint8 *new_state = SDL_GetKeyboardState(NULL);

    // If first call, initialize prevState to match the current state
    if (!this->keyboard_state) {
        this->keyboard_state = new Uint8[SDL_NUM_SCANCODES];
        memcpy((void *)this->keyboard_state, new_state, SDL_NUM_SCANCODES);
    }

    // Iterate over all scancodes and check for changes
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
        if (new_state[i] == this->keyboard_state[i]) {
            continue;
        }

        bool pressed = new_state[i];
        SDL_Scancode key = static_cast<SDL_Scancode>(i);

        for (Chord &chord : this->chords) {
            chord.ProcessKeyPress(key, pressed);
        }

        if (!this->IsKeyInChords(key)) {
            EventManager::GetInstance().RaiseInputEvent(
                InputEvent{InputEventType::Single, key, 0, pressed});
        }
    }

    for (Chord &chord : this->chords) {
        chord.Process();
    }

    // Update the previous state
    memcpy((void *)this->keyboard_state, new_state, SDL_NUM_SCANCODES);
}

void Input::RegisterInputChord(int chord_id, std::unordered_set<SDL_Scancode> keys) {
    this->chords.push_back(Chord(chord_id, keys));
}