#pragma once

enum Shape { Circle, Square, Rectangle, Triangle };
enum GameObjectCategory { Controllable, Moving, Stationary };

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
