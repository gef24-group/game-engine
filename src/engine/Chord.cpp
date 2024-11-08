#include "Chord.hpp"
#include "EventManager.hpp"
#include "Types.hpp"
#include "Utils.hpp"

Chord::Chord(int chord_id, std::unordered_set<SDL_Scancode> keys) {
    if (chord_id == 0) {
        Log(LogLevel::Error, "The chord_id should be greater than 0!");
        app->quit.store(true);
        return;
    }

    this->chord_id = chord_id;
    this->keys = keys;
    this->buffer = std::vector<std::pair<SDL_Scancode, bool>>();
    this->start = std::chrono::steady_clock::now();
    this->timeout = 50;
}

void Chord::Process() {
    auto now = std::chrono::steady_clock::now();
    this->elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - this->start).count();

    if (this->elapsed >= this->timeout) {
        this->Reset();
    }
}

void Chord::ProcessKeyPress(SDL_Scancode key, bool pressed) {
    if (!this->IsKeyInChord(key)) {
        return;
    }

    this->buffer.push_back({key, pressed});
    this->LogBuffer();

    if (this->BufferContainsChord()) {
        this->Trigger();
    }
}

bool Chord::BufferContainsChord() {
    std::vector<std::pair<SDL_Scancode, bool>> matched_entries;

    for (const auto &key : this->keys) {
        auto buffer_iterator = std::find_if(this->buffer.begin(), this->buffer.end(),
                                            [&key](const std::pair<SDL_Scancode, bool> &entry) {
                                                return entry.first == key && entry.second;
                                            });

        if (buffer_iterator == this->buffer.end()) {
            return false;
        }

        matched_entries.push_back(*buffer_iterator);
    }

    for (const auto &entry : matched_entries) {
        auto iterator = std::find(this->buffer.begin(), this->buffer.end(), entry);
        if (iterator != this->buffer.end()) {
            this->buffer.erase(iterator);
        }
    }

    return true;
}

void Chord::FlushBuffer() {
    for (const auto &entry : this->buffer) {
        SDL_Scancode key = entry.first;
        bool pressed = entry.second;

        EventManager::GetInstance().RaiseInputEvent(
            InputEvent{InputEventType::Single, key, 0, pressed});
    }
    this->buffer.clear();
}

void Chord::LogBuffer() {
    std::string log_buffer = "";
    for (const auto &entry : this->buffer) {
        SDL_Scancode key = entry.first;
        bool pressed = entry.second;

        log_buffer += std::to_string(key) + "_" + std::to_string(pressed) + " , ";
    }
    Log(LogLevel::Info, "buffer: %s", log_buffer.c_str());
}

void Chord::Reset() {
    this->start = std::chrono::steady_clock::now();
    this->FlushBuffer();
}

void Chord::Trigger() {
    this->Reset();

    EventManager::GetInstance().RaiseInputEvent(
        InputEvent{InputEventType::Chord, SDL_SCANCODE_UNKNOWN, this->chord_id, true});
}

bool Chord::IsKeyInChord(SDL_Scancode key) { return this->keys.count(key) > 0; }