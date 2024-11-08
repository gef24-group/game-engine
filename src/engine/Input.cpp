#include "Input.hpp"
#include "Chord.hpp"
#include "EventManager.hpp"
#include "SDL_keyboard.h"
#include "SDL_scancode.h"
#include "Types.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cstddef>

Input::Input() {
    this->keyboard_state = nullptr;
    this->chords = std::vector<Chord>();

    this->buffer = std::vector<std::pair<SDL_Scancode, bool>>();
    this->start = std::chrono::steady_clock::now();
    this->elapsed = 0;
    this->timeout = 100;
}

bool Input::IsKeyInChords(SDL_Scancode key) {
    for (Chord &chord : this->chords) {
        if (chord.IsKeyInChord(key)) {
            return true;
        }
    }
    return false;
}

void Input::LogBuffer() {
    std::string log_buffer = "";
    for (const auto &entry : this->buffer) {
        SDL_Scancode key = entry.first;
        bool pressed = entry.second;

        log_buffer += std::to_string(key) + "_" + std::to_string(pressed) + " , ";
    }
    Log(LogLevel::Info, "buffer: %s", log_buffer.c_str());
}

void Input::FlushBuffer() {
    for (const auto &entry : this->buffer) {
        SDL_Scancode key = entry.first;
        bool pressed = entry.second;

        EventManager::GetInstance().RaiseInputEvent(
            InputEvent{InputEventType::Single, key, 0, pressed});
    }
    this->buffer.clear();
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

        if (!this->IsKeyInChords(key)) {
            EventManager::GetInstance().RaiseInputEvent(
                InputEvent{InputEventType::Single, key, 0, pressed});
            continue;
        }

        this->buffer.push_back({key, pressed});

        int chord_id = this->BufferContainsChord();
        if (chord_id != 0) {
            this->TriggerChord(chord_id);
        }
    }

    auto now = std::chrono::steady_clock::now();
    this->elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - this->start).count();

    if (this->elapsed >= this->timeout) {
        this->ResetChords();
    }

    // Update the previous state
    memcpy((void *)this->keyboard_state, new_state, SDL_NUM_SCANCODES);
}

int Input::BufferContainsChord() {
    for (auto &chord : this->chords) {
        bool chord_found = true;

        for (const auto &key : chord.GetKeys()) {
            auto buffer_iterator =
                std::find_if(this->buffer.begin(), this->buffer.end(),
                             [key](const std::pair<SDL_Scancode, bool> &entry) {
                                 return entry.first == key && entry.second == true;
                             });
            if (buffer_iterator == buffer.end()) {
                chord_found = false;
                break;
            }
        }

        if (chord_found) {
            for (const auto &key : chord.GetKeys()) {
                auto buffer_iterator =
                    std::remove_if(buffer.begin(), buffer.end(),
                                   [key](const std::pair<SDL_Scancode, bool> &entry) {
                                       return entry.first == key && entry.second == true;
                                   });
                buffer.erase(buffer_iterator, buffer.end());
            }
            return chord.GetChordID();
        }
    }

    return 0;
}

void Input::TriggerChord(int chord_id) {
    this->ResetChords();

    EventManager::GetInstance().RaiseInputEvent(
        InputEvent{InputEventType::Chord, SDL_SCANCODE_UNKNOWN, chord_id, true});
}

void Input::ResetChords() {
    this->start = std::chrono::steady_clock::now();
    this->FlushBuffer();
}

void Input::RegisterInputChord(int chord_id, std::unordered_set<SDL_Scancode> keys) {
    this->chords.push_back(Chord(chord_id, keys));
}