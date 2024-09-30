#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <string>

enum Shape { Circle, Square, Rectangle, Triangle };
enum GameObjectCategory { Controllable, Moving, Stationary };
enum LogLevel { Verbose = 1, Debug, Info, Warn, Error, Critical, Priorities };
enum NetworkMode { Single, ClientServer, PeerToPeer };
enum NetworkRole { Server, Client, Host, Peer };

struct Color {
    int red;
    int green;
    int blue;
    int alpha = 255;
};

struct Border {
    bool show = false;
    Color color;
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
    std::atomic<bool> pressed = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_pressed_time;
    std::function<void()> OnPress = []() {};
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
    Key key_comma;
    Key key_period;
};

struct Window {
    int width = 1920;
    int height = 1080;
    bool proportional_scaling = true;
};

struct FrameTime {
    int64_t current;
    int64_t last;
    int64_t delta;
};

struct NetworkInfo {
    NetworkMode mode;
    NetworkRole role;
    int id = 0;
    std::string server_ip;
    std::string host_ip;
    std::string peer_ip;
};

struct JoinReply {
    int player_id;
};

struct ObjectUpdate {
    char name[100];
    Position position;
    char player_address[100];
};