#pragma once
#include <chrono>

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

struct Key {
    bool pressed = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_pressed_time;
};

struct KeyMap {
    Key key_W;
    Key key_A;
    Key key_S;
    Key key_D;
    Key key_X;
    Key key_P;
    Key key_up;
    Key key_left;
    Key key_down;
    Key key_right;
    Key key_space;
    Key key_less_than;
    Key key_greater_than;
};

struct Window {
    int width = 1920;
    int height = 1080;
    bool proportional_scaling = false;
};