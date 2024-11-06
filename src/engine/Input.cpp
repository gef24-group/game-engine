#include "Input.hpp"

#include "Engine.hpp"
#include "EventManager.hpp"
#include "SDL_keyboard.h"
#include "SDL_scancode.h"
#include "Types.hpp"
#include "Utils.hpp"
#include <cstddef>

Input::Input() {
    this->keyboard_state = nullptr;
    this->chords = std::unordered_map<int, std::unordered_set<SDL_Scancode>>();
    this->key_press_times = std::unordered_map<SDL_Scancode, int64_t>();
    this->chord_delay = 100;
}

// Event handler function to be called on key state change
void Input::RaiseEvent(SDL_Scancode key, bool is_pressed) {
    EventManager::GetInstance().RaiseInputEvent(
        InputEvent{InputEventType::Single, key, 0, is_pressed});
}

bool Input::IsKeyInChords(SDL_Scancode key) {
    for (const auto &[chord_id, chord_keys] : this->chords) {
        if (chord_keys.count(key) > 0) {
            return true;
        }
    }
    return false;
}

void Input::Process() {
    const Uint8 *new_state = SDL_GetKeyboardState(NULL);

    // If first call, initialize prevState to match the current state
    if (!this->keyboard_state) {
        int num_keys;
        this->keyboard_state = new Uint8[SDL_NUM_SCANCODES];
        memcpy((void *)this->keyboard_state, new_state, SDL_NUM_SCANCODES);
    }

    // To ensure only changes to chords are raised
    static std::unordered_map<int, bool> active_chords;

    int64_t current_time = Engine::GetInstance().EngineTimelineGetTime();

    for (const auto &[chord_id, chord_keys] : this->chords) {
        bool chord_detected = true;

        for (SDL_Scancode key : chord_keys) {
            if (!new_state[key]) {
                chord_detected = false;
                break;
            }
        }

        if (chord_detected) {
            if (!active_chords[chord_id]) {
                EventManager::GetInstance().RaiseInputEvent(
                    InputEvent{InputEventType::Chord, SDL_SCANCODE_UNKNOWN, chord_id, true});
                active_chords[chord_id] = true;
            }
        } else {
            if (active_chords[chord_id]) {
                EventManager::GetInstance().RaiseInputEvent(
                    InputEvent{InputEventType::Chord, SDL_SCANCODE_UNKNOWN, chord_id, false});
                active_chords[chord_id] = false;
            }
        }
    }

    // Iterate over all scancodes and check for changes
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
        if (new_state[i] == this->keyboard_state[i]) {
            continue;
        }

        bool is_pressed = new_state[i];
        SDL_Scancode key = static_cast<SDL_Scancode>(i);

        if (is_pressed) {
            key_press_times[key] = current_time;
        } else {
            key_press_times.erase(key);
        }

        if (this->IsKeyInChords(key)) {
            int64_t delay = Engine::GetInstance().EngineTimelineGetTime() - key_press_times[key];

            if (delay >= (this->chord_delay * 1'000'000)) {
                this->RaiseEvent(key, is_pressed);
            }
        } else {
            this->RaiseEvent(key, is_pressed);
        }
    }

    // Update the previous state
    memcpy((void *)this->keyboard_state, new_state, SDL_NUM_SCANCODES);
}

void Input::RegisterInputChord(int chord_id, std::unordered_set<SDL_Scancode> chord) {
    if (chord_id == 0) {
        Log(LogLevel::Error, "The chord_id should be greater than 0!");
        app->quit.store(true);
        return;
    }

    this->chords[chord_id] = chord;
}