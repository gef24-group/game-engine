#include "Chord.hpp"
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
}

int Chord::GetChordID() { return this->chord_id; }

std::unordered_set<SDL_Scancode> Chord::GetKeys() { return this->keys; }

bool Chord::IsKeyInChord(SDL_Scancode key) { return this->keys.count(key) > 0; }