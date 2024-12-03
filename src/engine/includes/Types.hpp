#pragma once

#include "SDL_scancode.h"
#include <string>
#include <variant>

class Entity;

enum class Shape { Circle, Square, Rectangle, Triangle };
enum class EntityCategory {
    Controllable,
    Moving,
    Stationary,
    SpawnPoint,
    DeathZone,
    SideBoundary,
    Camera
};
enum class LogLevel { Verbose = 1, Debug, Info, Warn, Error, Critical, Priorities };
enum class NetworkMode { Single, ClientServer, PeerToPeer };
enum class NetworkRole { Server, Client, Host, Peer };
enum class Direction { Horizontal, Vertical };
enum class Overlap { Left, Right, Top, Bottom, None };
enum class Encoding { Struct, JSON };

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

struct EntityUpdate {
    char name[100] = "";
    Position position;
    char player_address[100] = "";
    bool active = true;
};

enum class EventType {
    Input,
    Move,
    Collision,
    Spawn,
    Death,
    Join,
    Discover,
    Leave,
    StartRecord,
    StopRecord,
    StartReplay,
    StopReplay
};

enum class InputEventType { Single, Chord };

struct InputEvent {
    InputEventType type;
    SDL_Scancode key;
    int chord_id = 0;
    bool pressed;
};

struct CollisionEvent {
    Entity *collider_1;
    Entity *collider_2;
};

struct DeathEvent {
    Entity *entity;
};

struct SpawnEvent {
    Entity *entity;
};

struct MoveEvent {
    Entity *entity;
    Position position;
};

struct JoinEvent {
    char player_address[100] = "";
};

struct DiscoverEvent {};

struct LeaveEvent {};

struct StartRecordEvent {};

struct StopRecordEvent {};

struct StartReplayEvent {};

struct StopReplayEvent {};

typedef std::variant<InputEvent, CollisionEvent, DeathEvent, SpawnEvent, MoveEvent, JoinEvent,
                     DiscoverEvent, LeaveEvent, StartRecordEvent, StopRecordEvent, StartReplayEvent,
                     StopReplayEvent>
    EventData;

enum class Priority { High, Medium, Low };