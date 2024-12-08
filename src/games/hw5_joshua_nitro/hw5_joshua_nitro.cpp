#include "Collision.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "EventManager.hpp"
#include "Handler.hpp"
#include "Network.hpp"
#include "Physics.hpp"
#include "Render.hpp"
#include "SDL_scancode.h"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <cstddef>
#include <limits>
#include <random>
#include <string>
#include <unordered_set>

Entity *highest_track = nullptr;
Size racetrack_size = Size{1920, 2048};
Velocity racetrack_velocity = Velocity{0, 60};
Size car_size = Size{70, 122};
int64_t last_obstacle_time;
int64_t obstacle_time_threshold = static_cast<int64_t>(1'000'000 * 2000);
std::vector<std::pair<const char *, Size>> obstacle_textures = {{"barrel_1.png", Size{56, 56}},
                                                                {"barrel_2.png", Size{70, 48}},
                                                                {"cone.png", Size{46, 44}},
                                                                {"oil.png", Size{109, 95}},
                                                                {"rock.png", Size{87, 67}}};

Color background_color = Color{166, 201, 203, 255};
Size window_size;
NetworkInfo network_info;

struct KeyState {
    bool up;
    bool down;
    bool left;
    bool right;
    bool power_up;
    bool power_down;
    bool power_left;
    bool power_right;
} key_state;

struct PowerTime {
    int64_t up;
    int64_t down;
    int64_t left;
    int64_t right;
} power_time;

void Update(std::vector<Entity *> &entities) {
    std::vector<Entity *> outside_obstacles;
    for (const auto &entity : entities) {
        if (entity->GetName().find("obstacle") == 0 &&
            entity->GetComponent<Transform>()->GetPosition().x ==
                -float(entity->GetComponent<Transform>()->GetSize().width)) {
            outside_obstacles.push_back(entity);
        }
    }

    int64_t current_time = Engine::GetInstance().EngineTimelineGetFrameTime().current;
    bool include_obstacle = false;

    if (current_time - last_obstacle_time > obstacle_time_threshold) {
        include_obstacle = true;
    }

    if (include_obstacle && outside_obstacles.size() > 0) {
        last_obstacle_time = Engine::GetInstance().EngineTimelineGetFrameTime().current;

        std::random_device random_device;
        std::mt19937 eng(random_device());
        std::uniform_int_distribution<> obstacle_dist(0, int(outside_obstacles.size() - 1));
        size_t random_obstacle_index = obstacle_dist(eng);

        std::uniform_int_distribution<> pos_x_dist(300, 1495);
        int random_pos_x = pos_x_dist(eng);
        Entity *random_obstacle = outside_obstacles[random_obstacle_index];
        Position random_obstacle_pos = random_obstacle->GetComponent<Transform>()->GetPosition();
        random_obstacle_pos.x = float(random_pos_x);
        random_obstacle_pos.y =
            -float(random_obstacle->GetComponent<Transform>()->GetSize().height);

        EventManager::GetInstance().RaiseMoveEvent(MoveEvent{random_obstacle, random_obstacle_pos});
    }

    float highest_track_y = std::numeric_limits<float>::max();
    for (const auto &entity : entities) {
        if (entity->GetName().find("track") == 0) {
            float track_y = entity->GetComponent<Transform>()->GetPosition().y;
            if (track_y < highest_track_y) {
                highest_track = entity;
                highest_track_y = track_y;
            }
        }
    }
}

void HandleCarSingleInput(Entity &car, InputEvent *event) {
    bool pressed = event->pressed;

    SDL_Scancode key = event->key;
    switch (key) {
    case SDL_SCANCODE_W:
        key_state.up = pressed;
        break;

    case SDL_SCANCODE_S:
        key_state.down = pressed;
        break;

    case SDL_SCANCODE_A:
        key_state.left = pressed;
        break;

    case SDL_SCANCODE_D:
        key_state.right = pressed;
        break;

    default:
        break;
    }
}

void HandleCarChordInput(Entity &car, InputEvent *event) {
    bool pressed = event->pressed;

    if (!pressed) {
        return;
    }

    switch (event->chord_id) {
    case 1: {
        key_state.power_up = pressed;
        power_time.up = Engine::GetInstance().EngineTimelineGetFrameTime().current;
        break;
    }
    case 2: {
        key_state.power_down = pressed;
        power_time.down = Engine::GetInstance().EngineTimelineGetFrameTime().current;
        break;
    }
    case 3: {
        key_state.power_left = pressed;
        power_time.left = Engine::GetInstance().EngineTimelineGetFrameTime().current;
        break;
    }
    case 4: {
        key_state.power_right = pressed;
        power_time.right = Engine::GetInstance().EngineTimelineGetFrameTime().current;
        break;
    }
    default:
        break;
    }
}

void HandleCarInput(Entity &car, Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));
    if (input_event == nullptr) {
        return;
    }

    switch (input_event->type) {
    case InputEventType::Single:
        HandleCarSingleInput(car, input_event);
        break;
    case InputEventType::Chord:
        HandleCarChordInput(car, input_event);
        break;
    default:
        break;
    }
}

void HandleCarEvent(Entity &car, Event &event) {
    switch (event.type) {
    case EventType::Input:
        HandleCarInput(car, event);
        break;
    default:
        break;
    }
}

void TimedPowerDown() {
    int64_t current_time = Engine::GetInstance().EngineTimelineGetFrameTime().current;
    int64_t power_up_time = current_time - power_time.up;
    int64_t power_down_time = current_time - power_time.down;
    int64_t power_left_time = current_time - power_time.left;
    int64_t power_right_time = current_time - power_time.right;
    int64_t threshold_time = static_cast<int64_t>(1'000'000 * 800);

    if (key_state.power_up && power_up_time > threshold_time) {
        key_state.power_up = false;
    }
    if (key_state.power_down && power_down_time > threshold_time) {
        key_state.power_down = false;
    }
    if (key_state.power_left && power_left_time > threshold_time) {
        key_state.power_left = false;
    }
    if (key_state.power_right && power_right_time > threshold_time) {
        key_state.power_right = false;
    }
}

void UpdateCar(Entity &car) {
    TimedPowerDown();

    Velocity car_velocity = car.GetComponent<Physics>()->GetVelocity();
    double angle = car.GetComponent<Transform>()->GetAngle();
    float min_x_velocity = -55.0f;
    float max_x_velocity = 55.0f;
    float y_velocity_easing = 0.009f;
    float max_y_velocity_factor = 0.5f;

    if (key_state.up) {
        car_velocity.y -= 2;
    }
    if (key_state.down) {
        car_velocity.y += 2;
    }
    if (key_state.left) {
        car_velocity.x -= 1;
        angle -= 1;
    }
    if (key_state.right) {
        car_velocity.x += 1;
        angle += 1;
    }

    if (key_state.power_up) {
        car_velocity.y -= 6;
        max_y_velocity_factor = 0.8f;
        y_velocity_easing = 0.05f;
    }
    if (key_state.power_down) {
        car_velocity.y += 6;
        y_velocity_easing = 0.05f;
    }
    if (key_state.power_left) {
        car_velocity.x -= 4;
        min_x_velocity = -65.0f;
        max_x_velocity = 65.0f;
        angle -= 1;
    }
    if (key_state.power_right) {
        car_velocity.x += 4;
        min_x_velocity = -65.0f;
        max_x_velocity = 65.0f;
        angle += 1;
    }

    if (!key_state.up || !key_state.down) {
        car_velocity.y += (racetrack_velocity.y - car_velocity.y) * y_velocity_easing;
    }
    if (!key_state.left && !key_state.right && !key_state.power_left && !key_state.power_right) {
        car_velocity.x *= 0.8;
        angle *= 0.8;
    }

    car_velocity.x = std::clamp(car_velocity.x, min_x_velocity, max_x_velocity);
    car_velocity.y = std::clamp(car_velocity.y, -(racetrack_velocity.y * max_y_velocity_factor),
                                racetrack_velocity.y);
    angle = std::clamp(angle, -10.0, 10.0);

    car.GetComponent<Physics>()->SetVelocity(car_velocity);
    car.GetComponent<Transform>()->SetAngle(angle);
}

void UpdateObstacle(Entity &obstacle) {
    float obstacle_top_edge = obstacle.GetComponent<Transform>()->GetPosition().y;
    float map_bottom_edge = float(window_size.height);
    bool obstacle_out_bottom = obstacle_top_edge > map_bottom_edge;

    if (obstacle_out_bottom) {
        Position new_pos = Position{-float(obstacle.GetComponent<Transform>()->GetSize().width),
                                    -float(obstacle.GetComponent<Transform>()->GetSize().height)};
        EventManager::GetInstance().RaiseMoveEvent(MoveEvent{&obstacle, new_pos});
    }
}

void UpdateRaceTrack(Entity &track) {
    float track_top_edge = track.GetComponent<Transform>()->GetPosition().y;
    float map_bottom_edge = float(window_size.height);
    bool track_out_bottom = track_top_edge > map_bottom_edge;

    if (track_out_bottom) {
        Position new_pos = Position{highest_track->GetComponent<Transform>()->GetPosition().x,
                                    highest_track->GetComponent<Transform>()->GetPosition().y -
                                        float(racetrack_size.height)};
        EventManager::GetInstance().RaiseMoveEvent(MoveEvent{&track, new_pos});
    }
}

void CreateCar(std::vector<Entity *> &entities) {
    Entity *car = new Entity("car", EntityCategory::Controllable);
    car->AddComponent<Render>();
    car->AddComponent<Transform>();
    car->AddComponent<Physics>();
    car->AddComponent<Collision>();
    car->AddComponent<Handler>();
    car->AddComponent<Network>();

    car->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    car->GetComponent<Render>()->SetTextureTemplate("car_{}.png");
    car->GetComponent<Render>()->SetDepth(2);
    car->GetComponent<Transform>()->SetSize(car_size);
    car->GetComponent<Physics>()->SetVelocity(racetrack_velocity);
    car->GetComponent<Collision>()->SetAvoidTransform(true);
    car->GetComponent<Handler>()->SetEventCallback(HandleCarEvent);
    car->GetComponent<Handler>()->SetUpdateCallback(UpdateCar);
    car->GetComponent<Network>()->SetOwner(NetworkRole::Client);

    entities.push_back(car);
}

void CreateObstacles(std::vector<Entity *> &entities) {
    int obstacle_index = 0;

    for (const auto &[texture, size] : obstacle_textures) {
        Entity *obstacle =
            new Entity("obstacle_" + std::to_string(obstacle_index), EntityCategory::DeathZone);
        obstacle->AddComponent<Transform>();
        obstacle->AddComponent<Physics>();
        obstacle->AddComponent<Handler>();
        obstacle->AddComponent<Render>();
        obstacle->AddComponent<Network>();

        obstacle->GetComponent<Transform>()->SetPosition(
            Position{-float(size.width), -float(size.height)});
        obstacle->GetComponent<Transform>()->SetSize(size);
        obstacle->GetComponent<Physics>()->SetVelocity(racetrack_velocity);
        obstacle->GetComponent<Handler>()->SetUpdateCallback(UpdateObstacle);
        obstacle->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        obstacle->GetComponent<Render>()->SetTexture(texture);
        obstacle->GetComponent<Render>()->SetDepth(1);
        obstacle->GetComponent<Network>()->SetOwner(NetworkRole::Server);

        obstacle_index++;

        entities.push_back(obstacle);
    }
}

void CreateRaceTrack(std::vector<Entity *> &entities) {
    int track_index = 0;

    while (track_index < 2) {
        Entity *track = new Entity("track_" + std::to_string(track_index), EntityCategory::Moving);
        track->AddComponent<Transform>();
        track->AddComponent<Physics>();
        track->AddComponent<Handler>();
        track->AddComponent<Render>();
        track->AddComponent<Network>();

        track->GetComponent<Transform>()->SetPosition(Position{
            0, -float(racetrack_size.height) + float(track_index * racetrack_size.height)});
        track->GetComponent<Transform>()->SetSize(racetrack_size);
        track->GetComponent<Physics>()->SetVelocity(racetrack_velocity);
        track->GetComponent<Handler>()->SetUpdateCallback(UpdateRaceTrack);
        track->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        track->GetComponent<Render>()->SetTexture("racetrack.png");
        track->GetComponent<Network>()->SetOwner(NetworkRole::Server);

        track_index++;

        entities.push_back(track);
    }
}

std::vector<Entity *> CreateEntities() {
    std::vector<Entity *> entities = std::vector<Entity *>();

    CreateCar(entities);
    CreateObstacles(entities);
    CreateRaceTrack(entities);

    for (Entity *entity : entities) {
        if (network_info.mode == NetworkMode::PeerToPeer) {
            if (entity->GetComponent<Network>() == nullptr) {
                continue;
            }

            if (entity->GetComponent<Network>()->GetOwner() == NetworkRole::Server) {
                entity->GetComponent<Network>()->SetOwner(NetworkRole::Host);
            }
            if (entity->GetComponent<Network>()->GetOwner() == NetworkRole::Client) {
                entity->GetComponent<Network>()->SetOwner(NetworkRole::Peer);
            }
        }
    }

    return entities;
}

void CreateSpawnPoints() {
    float spawn_y = float(window_size.height) / 2.0f;

    Engine::GetInstance().AddSpawnPoint(Position{917, spawn_y}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{609, spawn_y}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{1225, spawn_y}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{300, spawn_y}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{1534, spawn_y}, Size{10, 10});
}

void CreateDeathZones() {
    Engine::GetInstance().AddDeathZone(Position{0, -float(window_size.height * 3)},
                                       Size{261, window_size.height * 7});
    Engine::GetInstance().AddDeathZone(Position{1644, -float(window_size.height * 3)},
                                       Size{276, window_size.height * 7});
    Engine::GetInstance().AddDeathZone(
        Position{-float(window_size.width * 3), -float(car_size.height + 10)},
        Size{window_size.width * 7, 10});
    Engine::GetInstance().AddDeathZone(
        Position{-float(window_size.width * 3), float(window_size.height + car_size.height)},
        Size{window_size.width * 7, 10});
}

void DestroyEntities(std::vector<Entity *> entities) {
    for (Entity *entity : entities) {
        delete entity;
    }
}

void BindEngineInputs() {
    Engine::GetInstance().BindPauseKey(SDL_SCANCODE_P);
    Engine::GetInstance().BindSpeedDownKey(SDL_SCANCODE_COMMA);
    Engine::GetInstance().BindSpeedUpKey(SDL_SCANCODE_PERIOD);
    Engine::GetInstance().BindDisplayScalingKey(SDL_SCANCODE_X);
    Engine::GetInstance().BindHiddenZoneKey(SDL_SCANCODE_Z);
    Engine::GetInstance().BindRecordKey(SDL_SCANCODE_Q);
    Engine::GetInstance().BindReplayKey(SDL_SCANCODE_R);
}

void RegisterInputChords() {
    Engine::GetInstance().RegisterInputChord(1, {SDL_SCANCODE_W, SDL_SCANCODE_P});
    Engine::GetInstance().RegisterInputChord(2, {SDL_SCANCODE_S, SDL_SCANCODE_P});
    Engine::GetInstance().RegisterInputChord(3, {SDL_SCANCODE_A, SDL_SCANCODE_P});
    Engine::GetInstance().RegisterInputChord(4, {SDL_SCANCODE_D, SDL_SCANCODE_P});
}

int main(int argc, char *args[]) {
    std::string title = "CSC581 HW5 Joshua's Nitro";

    if (!SetEngineCLIOptions(argc, args)) {
        return 1;
    }
    network_info = Engine::GetInstance().GetNetworkInfo();

    Engine::GetInstance().SetBackgroundColor(background_color);
    Engine::GetInstance().SetTitle(title);
    Engine::GetInstance().SetShowPlayerBorder(false);

    if (!Engine::GetInstance().Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    BindEngineInputs();
    RegisterInputChords();
    network_info = Engine::GetInstance().GetNetworkInfo();
    window_size = GetWindowSize();

    CreateSpawnPoints();
    CreateDeathZones();

    std::vector<Entity *> entities = CreateEntities();
    for (Entity *entity : entities) {
        Engine::GetInstance().AddEntity(entity);
    }

    Engine::GetInstance().SetPlayerTextures(5);
    Engine::GetInstance().SetCallback(Update);

    Engine::GetInstance().Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    DestroyEntities(entities);
    return 0;
}