#pragma once

enum Shape { Circle, Square, Rectangle, Triangle };
enum GameObjectCategory { Controllable, Moving, Stationary };
enum LogLevel { Verbose = 1, Debug, Info, Warn, Error, Critical, Priorities };

struct Color {
    int red;
    int green;
    int blue;
    int alpha = 255;
};

struct Position {
    float x;
    float y;
};

struct Size {
    int width;
    int height;
};

struct Velocity {
    float x;
    float y;
};

struct Acceleration {
    float x;
    float y;
};

struct KeyMap {
    bool key_W = false;
    bool key_A = false;
    bool key_S = false;
    bool key_D = false;
    bool key_up = false;
    bool key_left = false;
    bool key_down = false;
    bool key_right = false;
};