#include "Collision.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Handler.hpp"
#include "Network.hpp"
#include "Physics.hpp"
#include "Render.hpp"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <random>
#include <string>

const int TILE_SIZE = 100;
Size window_size;
NetworkInfo network_info;
std::vector<const char *> enemy_textures =
    std::vector({"assets/ladybug.png", "assets/mouse.png", "assets/worm.png", "assets/bee.png"});
Engine *engine_ptr;

void Update(std::vector<Entity *> *entities) {
    std::random_device random_device;
    std::mt19937 eng(random_device());
    std::uniform_int_distribution<> include_dist(0, 25);
    std::uniform_int_distribution<> enemy_dist(0, int(enemy_textures.size() - 1));

    bool include_enemy = include_dist(eng) == 12;
    size_t random_enemy = enemy_dist(eng);

    float max_ground_x = 0;
    Entity *max_ground = nullptr;
    for (Entity *entity : *entities) {
        if (entity->GetName().find("ground") == 0) {
            if (entity->GetComponent<Transform>()->GetPosition().x > max_ground_x) {
                max_ground = entity;
                max_ground_x = entity->GetComponent<Transform>()->GetPosition().x;
            }
        }
    }

    bool ground_repositioned = false;
    for (Entity *entity : *entities) {
        if (entity->GetName().find("ground") == 0) {
            int right_edge =
                static_cast<int>(std::round(entity->GetComponent<Transform>()->GetPosition().x)) +
                entity->GetComponent<Transform>()->GetSize().width;
            if (right_edge <= 0) {
                ground_repositioned = true;
                entity->GetComponent<Transform>()->SetPosition(
                    Position{max_ground_x + TILE_SIZE, float(window_size.height - TILE_SIZE)});
            }
        }
    }

    int enemy_index = 0;
    if (include_enemy && max_ground != nullptr && ground_repositioned) {
        for (Entity *entity : *entities) {
            if (entity->GetName().find("enemy") == 0) {
                if (enemy_index == random_enemy) {
                    entity->GetComponent<Physics>()->SetVelocity(Velocity{-50, 0});
                }
                enemy_index += 1;
            }
        }
    }

    float min_enemy_x = float(window_size.width * 2.0);
    Entity *min_enemy = nullptr;
    for (Entity *entity : *entities) {
        if (entity->GetName().find("enemy") == 0) {
            if (entity->GetComponent<Transform>()->GetPosition().x < min_enemy_x) {
                min_enemy = entity;
                min_enemy_x = entity->GetComponent<Transform>()->GetPosition().x;
            }
        }
    }
    if (min_enemy_x + TILE_SIZE <= 0 && min_enemy != nullptr) {
        min_enemy->GetComponent<Transform>()->SetPosition(Position{
            float(window_size.width + TILE_SIZE), float(window_size.height - (TILE_SIZE * 2))});
        min_enemy->GetComponent<Physics>()->SetVelocity(Velocity{0, 0});
    }
}

void Updateplayer(Entity *player) {
    if (app->key_map->key_up.pressed.load() || app->key_map->key_W.pressed.load()) {
        player->GetComponent<Physics>()->SetVelocity(
            {player->GetComponent<Physics>()->GetVelocity().x, -60});
    }

    if (app->key_map->key_down.pressed.load() || app->key_map->key_S.pressed.load()) {
        player->GetComponent<Physics>()->SetVelocity(
            {player->GetComponent<Physics>()->GetVelocity().x, 60});
    }

    if (app->key_map->key_left.pressed.load() || app->key_map->key_A.pressed.load()) {
        player->GetComponent<Physics>()->SetVelocity(
            {-60, player->GetComponent<Physics>()->GetVelocity().y});
    } else if (app->key_map->key_right.pressed.load() || app->key_map->key_D.pressed.load()) {
        player->GetComponent<Physics>()->SetVelocity(
            {60, player->GetComponent<Physics>()->GetVelocity().y});
    } else {
        player->GetComponent<Physics>()->SetVelocity(
            {0, player->GetComponent<Physics>()->GetVelocity().y});
    }

    if (player->GetName() == "player_" + std::to_string(network_info.id) &&
        player->GetComponent<Collision>()->GetColliders().size() > 0) {
        for (Entity *entity : player->GetComponent<Collision>()->GetColliders()) {
            if (entity->GetName().find("enemy") == 0) {
                Log(LogLevel::Info, "");
                Log(LogLevel::Info, "You lost :(");
                Log(LogLevel::Info, "");
                app->quit.store(true);
                break;
            }
        }
    }
}

void UpdatePlatform(Entity *platform) {
    float platform_right_edge = platform->GetComponent<Transform>()->GetPosition().x +
                                float(platform->GetComponent<Transform>()->GetSize().width);
    float platform_left_edge = platform->GetComponent<Transform>()->GetPosition().x;

    if (platform_right_edge >= float(window_size.width) &&
        platform->GetComponent<Physics>()->GetVelocity().x > 0) {
        platform->GetComponent<Physics>()->SetVelocity(
            {-std::abs(platform->GetComponent<Physics>()->GetVelocity().x),
             platform->GetComponent<Physics>()->GetVelocity().y});
    }
    if (platform_left_edge <= 0 && platform->GetComponent<Physics>()->GetVelocity().x < 0) {
        platform->GetComponent<Physics>()->SetVelocity(
            {std::abs(platform->GetComponent<Physics>()->GetVelocity().x),
             platform->GetComponent<Physics>()->GetVelocity().y});
    }
}

Entity *CreatePlayer() {
    Entity *player = new Entity("player", Controllable);
    player->AddComponent<Render>();
    player->AddComponent<Transform>();
    player->AddComponent<Physics>();
    player->AddComponent<Collision>();
    player->AddComponent<Handler>();
    player->AddComponent<Network>();

    player->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    player->GetComponent<Render>()->SetTextureTemplate("assets/player_{}.png");
    player->GetComponent<Transform>()->SetPosition(
        Position{20, float(window_size.height - (TILE_SIZE * 5))});
    player->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
    player->GetComponent<Physics>()->SetAcceleration(Acceleration{0, 15});
    player->GetComponent<Physics>()->SetVelocity(Velocity{0, 0});
    player->GetComponent<Collision>()->SetRestitution(0.5);
    player->GetComponent<Handler>()->SetCallback(Updateplayer);
    player->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    return player;
}

Entity *CreatePlatform1() {
    Entity *platform = new Entity("platform", Stationary);
    platform->AddComponent<Render>();
    platform->AddComponent<Transform>();
    platform->AddComponent<Physics>();
    platform->AddComponent<Collision>();
    platform->AddComponent<Handler>();
    platform->AddComponent<Network>();

    platform->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    platform->GetComponent<Render>()->SetTexture("assets/stone.png");
    platform->GetComponent<Transform>()->SetPosition(Position{40, TILE_SIZE * 5.5});
    platform->GetComponent<Transform>()->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->GetComponent<Physics>()->SetVelocity(Velocity{40, 0});
    platform->GetComponent<Handler>()->SetCallback(UpdatePlatform);
    platform->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    return platform;
}

Entity *CreatePlatform2() {
    Entity *platform = new Entity("platform", Stationary);
    platform->AddComponent<Render>();
    platform->AddComponent<Transform>();
    platform->AddComponent<Physics>();
    platform->AddComponent<Collision>();
    platform->AddComponent<Handler>();
    platform->AddComponent<Network>();

    platform->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    platform->GetComponent<Render>()->SetTexture("assets/stone.png");
    platform->GetComponent<Transform>()->SetPosition(Position{26, TILE_SIZE * 5.5});
    platform->GetComponent<Transform>()->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->GetComponent<Physics>()->SetVelocity(Velocity{40, 0});
    platform->GetComponent<Handler>()->SetCallback(UpdatePlatform);
    platform->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    return platform;
}

Entity *CreatePlatform3() {
    Entity *platform = new Entity("platform", Stationary);
    platform->AddComponent<Render>();
    platform->AddComponent<Transform>();
    platform->AddComponent<Physics>();
    platform->AddComponent<Collision>();
    platform->AddComponent<Handler>();
    platform->AddComponent<Network>();

    platform->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    platform->GetComponent<Render>()->SetTexture("assets/stone.png");
    platform->GetComponent<Transform>()->SetPosition(Position{20, TILE_SIZE * 5.5});
    platform->GetComponent<Transform>()->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->GetComponent<Physics>()->SetVelocity(Velocity{40, 0});
    platform->GetComponent<Handler>()->SetCallback(UpdatePlatform);
    platform->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    return platform;
}

Entity *CreatePlatform4() {
    Entity *platform = new Entity("platform", Moving);
    platform->AddComponent<Render>();
    platform->AddComponent<Transform>();
    platform->AddComponent<Physics>();
    platform->AddComponent<Collision>();
    platform->AddComponent<Handler>();
    platform->AddComponent<Network>();

    platform->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    platform->GetComponent<Render>()->SetTexture("assets/stone.png");
    platform->GetComponent<Transform>()->SetPosition(Position{20, TILE_SIZE * 5.5});
    platform->GetComponent<Transform>()->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->GetComponent<Physics>()->SetVelocity(Velocity{40, 0});
    platform->GetComponent<Handler>()->SetCallback(UpdatePlatform);
    platform->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    return platform;
}

Entity *CreatePlatform5() {
    Entity *platform = new Entity("platform", Moving);
    platform->AddComponent<Render>();
    platform->AddComponent<Transform>();
    platform->AddComponent<Physics>();
    platform->AddComponent<Collision>();
    platform->AddComponent<Handler>();
    platform->AddComponent<Network>();

    platform->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    platform->GetComponent<Render>()->SetTexture("assets/stone.png");
    platform->GetComponent<Transform>()->SetPosition(Position{20, TILE_SIZE * 5.5});
    platform->GetComponent<Transform>()->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->GetComponent<Physics>()->SetVelocity(Velocity{40, 0});
    platform->GetComponent<Handler>()->SetCallback(UpdatePlatform);
    platform->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    return platform;
}

std::vector<Entity *> CreateGround() {
    std::vector<Entity *> ground;
    for (int i = 0; i < (window_size.width / TILE_SIZE) + 2; i++) {
        Entity *ground_tile = new Entity("ground_" + std::to_string(i), Moving);
        ground_tile->AddComponent<Render>();
        ground_tile->AddComponent<Transform>();
        ground_tile->AddComponent<Physics>();
        ground_tile->AddComponent<Collision>();
        ground_tile->AddComponent<Network>();
        ground_tile->AddComponent<Handler>();

        ground_tile->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        ground_tile->GetComponent<Transform>()->SetPosition(
            Position{float(i * TILE_SIZE), float(window_size.height - TILE_SIZE)});
        ground_tile->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
        ground_tile->GetComponent<Physics>()->SetVelocity(Velocity{-50, 0});
        ground_tile->GetComponent<Render>()->SetTexture("assets/ground.png");
        ground_tile->GetComponent<Network>()->SetOwner(NetworkRole::Server);
        ground.push_back(ground_tile);
    }
    return ground;
}

std::vector<Entity *> CreateEnemies() {
    std::vector<Entity *> enemies;
    int enemy_index = 0;
    for (const char *enemy_texture : enemy_textures) {
        Entity *enemy = new Entity("enemy_" + std::to_string(enemy_index), Moving);
        enemy->AddComponent<Transform>();
        enemy->AddComponent<Physics>();
        enemy->AddComponent<Render>();
        enemy->AddComponent<Collision>();
        enemy->AddComponent<Network>();
        enemy->AddComponent<Handler>();

        enemy->GetComponent<Transform>()->SetPosition(Position{
            float(window_size.width + TILE_SIZE), float(window_size.height - (TILE_SIZE * 2))});
        enemy->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
        enemy->GetComponent<Physics>()->SetVelocity(Velocity{0, 0});
        enemy->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        enemy->GetComponent<Render>()->SetTexture(enemy_texture);
        enemy->GetComponent<Network>()->SetOwner(NetworkRole::Server);
        enemies.push_back(enemy);
        enemy_index += 1;
    }
    return enemies;
}

std::vector<Entity *> CreateEntities() {
    Entity *player = CreatePlayer();
    Entity *platform1 = CreatePlatform1();
    Entity *platform2 = CreatePlatform2();
    Entity *platform3 = CreatePlatform3();
    Entity *platform4 = CreatePlatform4();
    Entity *platform5 = CreatePlatform5();
    // std::vector<Entity *> clouds = CreateClouds();
    // std::vector<Entity *> ground = CreateGround();
    // std::vector<Entity *> enemies = CreateEnemies();

    std::vector<Entity *> entities =
        std::vector({player, platform1, platform2, platform3, platform4, platform5});

    for (Entity *entity : entities) {
        if (network_info.mode == NetworkMode::PeerToPeer) {
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

void DestroyEntities(std::vector<Entity *> entities) {
    for (Entity *entity : entities) {
        delete entity;
    }
}

void SetupTimelineInputs(Engine *engine) {
    // toggle constant and proportional scaling
    app->key_map->key_X.OnPress = []() {
        app->window.proportional_scaling = !app->window.proportional_scaling;
    };
    // toggle pause or unpause
    app->key_map->key_P.OnPress = [engine]() { engine->BaseTimelineTogglePause(); };
    // slow down the timeline
    app->key_map->key_comma.OnPress = [engine]() {
        engine->BaseTimelineChangeTic(std::min(engine->BaseTimelineGetTic() * 2.0, 2.0));
    };
    // speed up the timeline
    app->key_map->key_period.OnPress = [engine]() {
        engine->BaseTimelineChangeTic(std::max(engine->BaseTimelineGetTic() / 2.0, 0.5));
    };
}

int main(int argc, char *args[]) {
    std::string title = "CSC581 HW3 Mitesh's Game";

    Engine engine;
    engine_ptr = &engine;
    if (!SetEngineCLIOptions(&engine, argc, args)) {
        return 1;
    }
    network_info = engine.GetNetworkInfo();

    Color background_color = Color{0, 0, 0, 255};
    engine.SetBackgroundColor(background_color);
    engine.SetTitle(title);
    engine.SetShowPlayerBorder(true);

    if (!engine.Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    SetupTimelineInputs(engine_ptr);
    network_info = engine.GetNetworkInfo();
    window_size = GetWindowSize();

    std::vector<Entity *> entities = CreateEntities();
    engine.SetPlayerTextures(5);
    engine.SetCallback(Update);

    engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    DestroyEntities(entities);
    return 0;
}