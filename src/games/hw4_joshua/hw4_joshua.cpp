#include "Collision.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Handler.hpp"
#include "Network.hpp"
#include "Physics.hpp"
#include "Render.hpp"
#include "SDL_scancode.h"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <string>
#include <unordered_set>

const int TILE_SIZE = 100;
Size window_size;
NetworkInfo network_info;
std::vector<const char *> enemy_textures =
    std::vector({"assets/ladybug.png", "assets/mouse.png", "assets/worm.png", "assets/bee.png"});

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

void Update(std::vector<Entity *> &entities) {}

void HandleAlienSingleInput(InputEvent *event) {
    bool pressed = event->pressed;

    SDL_Scancode key = event->key;
    switch (key) {
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_UP:
        key_state.up = pressed;
        break;

    case SDL_SCANCODE_S:
    case SDL_SCANCODE_DOWN:
        key_state.down = pressed;
        break;

    case SDL_SCANCODE_A:
    case SDL_SCANCODE_LEFT:
        key_state.left = pressed;
        break;

    case SDL_SCANCODE_D:
    case SDL_SCANCODE_RIGHT:
        key_state.right = pressed;
        break;

    default:
        break;
    }
}

void HandleAlienChordInput(Entity &alien, InputEvent *event) {
    bool pressed = event->pressed;

    switch (event->chord_id) {
    case 1:
    case 5:
        key_state.power_up = pressed;
        alien.GetComponent<Physics>()->SetVelocity(
            {alien.GetComponent<Physics>()->GetVelocity().x, -150});
        break;
    case 2:
    case 6:
        key_state.power_down = pressed;
        alien.GetComponent<Physics>()->SetVelocity(
            {alien.GetComponent<Physics>()->GetVelocity().x, 150});
        break;
    case 3:
    case 7:
        key_state.power_left = pressed;
        alien.GetComponent<Physics>()->SetVelocity(
            {-150, alien.GetComponent<Physics>()->GetVelocity().y});
        break;
    case 4:
    case 8:
        key_state.power_right = pressed;
        alien.GetComponent<Physics>()->SetVelocity(
            {150, alien.GetComponent<Physics>()->GetVelocity().y});
        break;
    default:
        break;
    }
}

void HandleAlienInput(Entity &alien, Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));
    if (input_event == nullptr) {
        return;
    }

    switch (input_event->type) {
    case InputEventType::Single:
        HandleAlienSingleInput(input_event);
        break;
    case InputEventType::Chord:
        HandleAlienChordInput(alien, input_event);
        break;
    default:
        break;
    }
}

void HandleAlienCollision(Entity &alien, Event &event) {
    CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));
    if (collision_event == nullptr) {
        return;
    }

    Entity *collider = nullptr;
    if (alien.GetName() == collision_event->collider_1->GetName()) {
        collider = collision_event->collider_2;
    } else if (alien.GetName() == collision_event->collider_2->GetName()) {
        collider = collision_event->collider_1;
    }
    if (collider == nullptr) {
        return;
    }

    if (collider->GetName().find("enemy") == 0) {
        Log(LogLevel::Info, "");
        Log(LogLevel::Info, "You lost :(");
        Log(LogLevel::Info, "");
        app->quit.store(true);
    }
    if (collider->GetName().find("house") == 0) {
        Log(LogLevel::Info, "");
        Log(LogLevel::Info, "You made it home!");
        Log(LogLevel::Info, "");
        app->quit.store(true);
    }
}

void HandleAlienEvent(Entity &alien, Event &event) {
    switch (event.type) {
    case EventType::Input:
        HandleAlienInput(alien, event);
        break;
    case EventType::Collision:
        HandleAlienCollision(alien, event);
        break;
    default:
        break;
    }
}

void UpdateAlien(Entity &alien) {
    if (key_state.up) {
        alien.GetComponent<Physics>()->SetVelocity(
            {alien.GetComponent<Physics>()->GetVelocity().x, -60});
    }
    if (key_state.down) {
        alien.GetComponent<Physics>()->SetVelocity(
            {alien.GetComponent<Physics>()->GetVelocity().x, 60});
    }
    if (key_state.left) {
        alien.GetComponent<Physics>()->SetVelocity(
            {-60, alien.GetComponent<Physics>()->GetVelocity().y});
    }
    if (key_state.right) {
        alien.GetComponent<Physics>()->SetVelocity(
            {60, alien.GetComponent<Physics>()->GetVelocity().y});
    }
    if (!key_state.left && !key_state.right) {
        alien.GetComponent<Physics>()->SetVelocity(
            {0, alien.GetComponent<Physics>()->GetVelocity().y});
    }
}

void UpdatePlatform(Entity &platform) {
    float platform_right_edge = platform.GetComponent<Transform>()->GetPosition().x +
                                float(platform.GetComponent<Transform>()->GetSize().width);
    float platform_left_edge = platform.GetComponent<Transform>()->GetPosition().x;

    if (platform_right_edge >= 2600 && platform.GetComponent<Physics>()->GetVelocity().x > 0) {
        platform.GetComponent<Physics>()->SetVelocity(
            {-std::abs(platform.GetComponent<Physics>()->GetVelocity().x),
             platform.GetComponent<Physics>()->GetVelocity().y});
    }
    if (platform_left_edge <= 880 && platform.GetComponent<Physics>()->GetVelocity().x < 0) {
        platform.GetComponent<Physics>()->SetVelocity(
            {std::abs(platform.GetComponent<Physics>()->GetVelocity().x),
             platform.GetComponent<Physics>()->GetVelocity().y});
    }
}

void UpdateEnemy(Entity &enemy) {
    float enemy_bottom_edge = enemy.GetComponent<Transform>()->GetPosition().y +
                              float(enemy.GetComponent<Transform>()->GetSize().height);
    float enemy_top_edge = enemy.GetComponent<Transform>()->GetPosition().y;

    if (enemy_bottom_edge >= float(window_size.height - TILE_SIZE) &&
        enemy.GetComponent<Physics>()->GetVelocity().y > 0) {
        enemy.GetComponent<Physics>()->SetVelocity({
            enemy.GetComponent<Physics>()->GetVelocity().x,
            -std::abs(enemy.GetComponent<Physics>()->GetVelocity().y),
        });
    }
    if (enemy_top_edge <= float(window_size.height - (TILE_SIZE * 5)) &&
        enemy.GetComponent<Physics>()->GetVelocity().y < 0) {
        enemy.GetComponent<Physics>()->SetVelocity({
            enemy.GetComponent<Physics>()->GetVelocity().x,
            std::abs(enemy.GetComponent<Physics>()->GetVelocity().y),
        });
    }
}

Entity *CreateAlien() {
    Entity *alien = new Entity("alien", EntityCategory::Controllable);
    alien->AddComponent<Render>();
    alien->AddComponent<Transform>();
    alien->AddComponent<Physics>();
    alien->AddComponent<Collision>();
    alien->AddComponent<Handler>();
    alien->AddComponent<Network>();

    alien->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    alien->GetComponent<Render>()->SetTextureTemplate("assets/alien_{}.png");
    alien->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
    alien->GetComponent<Physics>()->SetAcceleration(Acceleration{0, 15});
    alien->GetComponent<Physics>()->SetVelocity(Velocity{0, 0});
    alien->GetComponent<Collision>()->SetRestitution(0.5);
    alien->GetComponent<Handler>()->SetEventCallback(HandleAlienEvent);
    alien->GetComponent<Handler>()->SetUpdateCallback(UpdateAlien);
    alien->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    return alien;
}

Entity *CreatePlatform() {
    Entity *platform = new Entity("platform", EntityCategory::Moving);
    platform->AddComponent<Render>();
    platform->AddComponent<Transform>();
    platform->AddComponent<Physics>();
    platform->AddComponent<Handler>();
    platform->AddComponent<Network>();

    platform->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    platform->GetComponent<Render>()->SetTexture("assets/stone.png");
    platform->GetComponent<Transform>()->SetPosition(Position{20, TILE_SIZE * 5});
    platform->GetComponent<Transform>()->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->GetComponent<Physics>()->SetVelocity(Velocity{40, 0});
    platform->GetComponent<Handler>()->SetUpdateCallback(UpdatePlatform);
    platform->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    return platform;
}

Entity *CreateHouse() {
    Entity *house = new Entity("house", EntityCategory::Stationary);
    house->AddComponent<Render>();
    house->AddComponent<Transform>();
    house->AddComponent<Network>();

    Size house_size = Size{241, 217};
    house_size.height *= 2;
    house_size.width *= 2;
    house->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    house->GetComponent<Render>()->SetTexture("assets/house.png");
    house->GetComponent<Transform>()->SetPosition(
        Position{float(window_size.width * 2) - float(house_size.width) - 50,
                 float(window_size.height - (TILE_SIZE + house_size.height))});
    house->GetComponent<Transform>()->SetSize(house_size);
    house->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    return house;
}

Entity *CreateSun() {
    Entity *sun = new Entity("sun", EntityCategory::Stationary);
    sun->AddComponent<Render>();
    sun->AddComponent<Transform>();
    sun->AddComponent<Network>();

    Size sun_size = Size{84, 84};
    sun_size.height += 80;
    sun_size.width += 80;
    sun->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    sun->GetComponent<Render>()->SetTexture("assets/sun.png");
    sun->GetComponent<Transform>()->SetPosition(
        Position{float(window_size.width) / 2 - 900, (TILE_SIZE - 100)});
    sun->GetComponent<Transform>()->SetSize(sun_size);
    sun->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    return sun;
}

std::vector<Entity *> CreateGround() {
    std::vector<Entity *> ground;
    for (int i = 0; i < (window_size.width / TILE_SIZE) * 2; i++) {
        Entity *ground_tile = new Entity("ground_" + std::to_string(i), EntityCategory::Stationary);
        ground_tile->AddComponent<Render>();
        ground_tile->AddComponent<Transform>();
        ground_tile->AddComponent<Network>();
        ground_tile->AddComponent<Handler>();

        ground_tile->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        ground_tile->GetComponent<Transform>()->SetPosition(
            Position{float(i * TILE_SIZE), float(window_size.height - TILE_SIZE)});
        ground_tile->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
        ground_tile->GetComponent<Render>()->SetTexture("assets/ground.png");
        ground_tile->GetComponent<Network>()->SetOwner(NetworkRole::Client);
        ground.push_back(ground_tile);
    }
    return ground;
}

std::vector<Entity *> CreateClouds() {
    Entity *cloud_1 = new Entity("cloud_1", EntityCategory::Stationary);
    cloud_1->AddComponent<Render>();
    cloud_1->AddComponent<Transform>();
    cloud_1->AddComponent<Network>();

    cloud_1->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    cloud_1->GetComponent<Render>()->SetTexture("assets/cloud_1.png");
    cloud_1->GetComponent<Transform>()->SetPosition(Position{460, (TILE_SIZE * 1.5)});
    cloud_1->GetComponent<Transform>()->SetSize(Size{203, 121});
    cloud_1->GetComponent<Network>()->SetOwner(NetworkRole::Client);

    Entity *cloud_2 = new Entity("cloud_2", EntityCategory::Stationary);
    cloud_2->AddComponent<Render>();
    cloud_2->AddComponent<Transform>();
    cloud_2->AddComponent<Network>();

    cloud_2->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    cloud_2->GetComponent<Render>()->SetTexture("assets/cloud_2.png");
    cloud_2->GetComponent<Transform>()->SetPosition(Position{1260, (TILE_SIZE * 1.5)});
    cloud_2->GetComponent<Transform>()->SetSize(Size{216, 139});
    cloud_2->GetComponent<Network>()->SetOwner(NetworkRole::Client);

    Entity *cloud_3 = new Entity("cloud_3", EntityCategory::Stationary);
    cloud_3->AddComponent<Render>();
    cloud_3->AddComponent<Transform>();
    cloud_3->AddComponent<Network>();

    cloud_3->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    cloud_3->GetComponent<Render>()->SetTexture("assets/cloud_1.png");
    cloud_3->GetComponent<Transform>()->SetPosition(Position{2060, (TILE_SIZE * 1.5)});
    cloud_3->GetComponent<Transform>()->SetSize(Size{203, 121});
    cloud_3->GetComponent<Network>()->SetOwner(NetworkRole::Client);

    Entity *cloud_4 = new Entity("cloud_4", EntityCategory::Stationary);
    cloud_4->AddComponent<Render>();
    cloud_4->AddComponent<Transform>();
    cloud_4->AddComponent<Network>();

    cloud_4->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    cloud_4->GetComponent<Render>()->SetTexture("assets/cloud_2.png");
    cloud_4->GetComponent<Transform>()->SetPosition(Position{2860, (TILE_SIZE * 1.5)});
    cloud_4->GetComponent<Transform>()->SetSize(Size{203, 121});
    cloud_4->GetComponent<Network>()->SetOwner(NetworkRole::Client);

    return std::vector({cloud_1, cloud_2, cloud_3, cloud_4});
}

std::vector<Entity *> CreateEnemies() {
    std::vector<Entity *> enemies;
    int enemy_index = 0;
    float pos_x = 220.0f;
    float pos_y = float(window_size.height - (TILE_SIZE * 2));

    while (pos_x < 3000) {
        Entity *enemy = new Entity("enemy_" + std::to_string(enemy_index), EntityCategory::Moving);
        enemy->AddComponent<Transform>();
        enemy->AddComponent<Physics>();
        enemy->AddComponent<Handler>();
        enemy->AddComponent<Render>();
        enemy->AddComponent<Network>();

        float pos_y_offset = enemy_index % 2 == 0 ? -(TILE_SIZE * 3) : 0;
        float vel_y = enemy_index % 2 == 0 ? 30 : -30;

        enemy->GetComponent<Transform>()->SetPosition(Position{pos_x, pos_y + pos_y_offset});
        enemy->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
        enemy->GetComponent<Physics>()->SetVelocity(Velocity{0, vel_y});
        enemy->GetComponent<Handler>()->SetUpdateCallback(UpdateEnemy);
        enemy->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        enemy->GetComponent<Render>()->SetTexture(
            enemy_textures[enemy_index % enemy_textures.size()]);
        enemy->GetComponent<Network>()->SetOwner(NetworkRole::Server);
        enemies.push_back(enemy);

        enemy_index += 1;
        pos_x += 660;
    }
    return enemies;
}

std::vector<Entity *> CreateEntities() {
    Entity *alien = CreateAlien();
    Entity *platform = CreatePlatform();
    Entity *house = CreateHouse();
    Entity *sun = CreateSun();
    std::vector<Entity *> ground = CreateGround();
    std::vector<Entity *> clouds = CreateClouds();
    std::vector<Entity *> enemies = CreateEnemies();

    std::vector<Entity *> entities = std::vector({alien, platform, house, sun});
    for (Entity *ground_tile : ground) {
        entities.push_back(ground_tile);
    }
    for (Entity *cloud : clouds) {
        entities.push_back(cloud);
    }
    for (Entity *enemy : enemies) {
        entities.push_back(enemy);
    }

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

void CreateSideBoundaries() {
    Engine::GetInstance().AddSideBoundary(Position{500, -float(window_size.height)},
                                          Size{10, window_size.height * 3});
    Engine::GetInstance().AddSideBoundary(Position{1410, -float(window_size.height)},
                                          Size{10, window_size.height * 3});
}

void CreateSpawnPoints() {
    Engine::GetInstance().AddSpawnPoint(Position{530, float(window_size.height - (TILE_SIZE * 3))},
                                        Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{530, 40}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{1290, 40}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{1290, float(window_size.height - (TILE_SIZE * 3))},
                                        Size{10, 10});
}

void CreateDeathZones() {
    Engine::GetInstance().AddDeathZone(Position{-10, -float(window_size.height)},
                                       Size{10, window_size.height * 3});
    Engine::GetInstance().AddDeathZone(Position{3800, -float(window_size.height)},
                                       Size{10, window_size.height * 3});
    Engine::GetInstance().AddDeathZone(Position{-float(window_size.width), -float(TILE_SIZE * 3)},
                                       Size{window_size.width * 4, 10});
    Engine::GetInstance().AddDeathZone(
        Position{-float(window_size.width), float(window_size.height - 10)},
        Size{window_size.width * 4, 10});
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
}

void RegisterInputChords() {
    Engine::GetInstance().RegisterInputChord(1, {SDL_SCANCODE_A, SDL_SCANCODE_D});

    // Engine::GetInstance().RegisterInputChord(1, {SDL_SCANCODE_W, SDL_SCANCODE_SPACE});
    // Engine::GetInstance().RegisterInputChord(2, {SDL_SCANCODE_S, SDL_SCANCODE_SPACE});
    // Engine::GetInstance().RegisterInputChord(3, {SDL_SCANCODE_A, SDL_SCANCODE_SPACE});
    // Engine::GetInstance().RegisterInputChord(4, {SDL_SCANCODE_D, SDL_SCANCODE_SPACE});
    // Engine::GetInstance().RegisterInputChord(5, {SDL_SCANCODE_UP, SDL_SCANCODE_SPACE});
    // Engine::GetInstance().RegisterInputChord(6, {SDL_SCANCODE_DOWN, SDL_SCANCODE_SPACE});
    // Engine::GetInstance().RegisterInputChord(7, {SDL_SCANCODE_LEFT, SDL_SCANCODE_SPACE});
    // Engine::GetInstance().RegisterInputChord(8, {SDL_SCANCODE_RIGHT, SDL_SCANCODE_SPACE});
}

int main(int argc, char *args[]) {
    std::string title = "CSC581 HW4 Joshua's Game";

    if (!SetEngineCLIOptions(argc, args)) {
        return 1;
    }
    network_info = Engine::GetInstance().GetNetworkInfo();

    Color background_color = Color{52, 153, 219, 255};
    Engine::GetInstance().SetBackgroundColor(background_color);
    Engine::GetInstance().SetTitle(title);
    Engine::GetInstance().SetShowPlayerBorder(true);

    if (!Engine::GetInstance().Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    BindEngineInputs();
    RegisterInputChords();
    network_info = Engine::GetInstance().GetNetworkInfo();
    window_size = GetWindowSize();

    CreateSideBoundaries();
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