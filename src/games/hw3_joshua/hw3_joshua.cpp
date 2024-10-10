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

void UpdateAlien(Entity *alien) {
    if (app->key_map->key_up.pressed.load() || app->key_map->key_W.pressed.load()) {
        alien->GetComponent<Physics>()->SetVelocity(
            {alien->GetComponent<Physics>()->GetVelocity().x, -60});
    }

    if (app->key_map->key_down.pressed.load() || app->key_map->key_S.pressed.load()) {
        alien->GetComponent<Physics>()->SetVelocity(
            {alien->GetComponent<Physics>()->GetVelocity().x, 60});
    }

    if (app->key_map->key_left.pressed.load() || app->key_map->key_A.pressed.load()) {
        alien->GetComponent<Physics>()->SetVelocity(
            {-60, alien->GetComponent<Physics>()->GetVelocity().y});
    } else if (app->key_map->key_right.pressed.load() || app->key_map->key_D.pressed.load()) {
        alien->GetComponent<Physics>()->SetVelocity(
            {60, alien->GetComponent<Physics>()->GetVelocity().y});
    } else {
        alien->GetComponent<Physics>()->SetVelocity(
            {0, alien->GetComponent<Physics>()->GetVelocity().y});
    }

    if (alien->GetName() == "alien_" + std::to_string(network_info.id) &&
        alien->GetComponent<Collision>()->GetColliders().size() > 0) {
        for (Entity *entity : alien->GetComponent<Collision>()->GetColliders()) {
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

Entity *CreateAlien() {
    Entity *alien = new Entity("alien", Controllable);
    alien->AddComponent<Render>();
    alien->AddComponent<Transform>();
    alien->AddComponent<Physics>();
    alien->AddComponent<Collision>();
    alien->AddComponent<Handler>();
    alien->AddComponent<Network>();

    alien->GetComponent<Physics>()->SetEngineTimeline(engine_ptr->GetBaseTimeline());

    alien->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    alien->GetComponent<Render>()->SetTextureTemplate("assets/alien_{}.png");
    alien->GetComponent<Transform>()->SetPosition(
        Position{20, float(window_size.height - (TILE_SIZE * 5))});
    alien->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
    alien->GetComponent<Physics>()->SetAcceleration(Acceleration{0, 15});
    alien->GetComponent<Physics>()->SetVelocity(Velocity{0, 0});
    alien->GetComponent<Collision>()->SetRestitution(0.5);
    alien->GetComponent<Handler>()->SetCallback(UpdateAlien);
    alien->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    return alien;
}

Entity *CreatePlatform() {
    Entity *platform = new Entity("platform", Moving);
    platform->AddComponent<Render>();
    platform->AddComponent<Transform>();
    platform->AddComponent<Physics>();
    platform->AddComponent<Collision>();
    platform->AddComponent<Handler>();
    platform->AddComponent<Network>();

    platform->GetComponent<Physics>()->SetEngineTimeline(engine_ptr->GetBaseTimeline());

    platform->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    platform->GetComponent<Render>()->SetTexture("assets/stone.png");
    platform->GetComponent<Transform>()->SetPosition(Position{20, TILE_SIZE * 5.5});
    platform->GetComponent<Transform>()->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->GetComponent<Physics>()->SetVelocity(Velocity{40, 0});
    platform->GetComponent<Collision>()->SetAffectedByCollision(false);
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

        ground_tile->GetComponent<Physics>()->SetEngineTimeline(engine_ptr->GetBaseTimeline());

        ground_tile->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        ground_tile->GetComponent<Transform>()->SetPosition(
            Position{float(i * TILE_SIZE), float(window_size.height - TILE_SIZE)});
        ground_tile->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
        ground_tile->GetComponent<Physics>()->SetVelocity(Velocity{-50, 0});
        ground_tile->GetComponent<Collision>()->SetAffectedByCollision(false);
        ground_tile->GetComponent<Render>()->SetTexture("assets/ground.png");
        ground_tile->GetComponent<Network>()->SetOwner(NetworkRole::Server);
        ground.push_back(ground_tile);
    }
    return ground;
}

std::vector<Entity *> CreateClouds() {
    Entity *cloud_1 = new Entity("cloud_1", Stationary);
    cloud_1->AddComponent<Render>();
    cloud_1->AddComponent<Transform>();
    cloud_1->AddComponent<Collision>();
    cloud_1->AddComponent<Network>();
    cloud_1->AddComponent<Physics>();
    cloud_1->AddComponent<Handler>();

    cloud_1->GetComponent<Physics>()->SetEngineTimeline(engine_ptr->GetBaseTimeline());

    cloud_1->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    cloud_1->GetComponent<Render>()->SetTexture("assets/cloud_1.png");
    cloud_1->GetComponent<Transform>()->SetPosition(
        Position{float(window_size.width) / 2 - 500, (TILE_SIZE * 1.5)});
    cloud_1->GetComponent<Transform>()->SetSize(Size{203, 121});
    cloud_1->GetComponent<Collision>()->SetAffectedByCollision(false);
    cloud_1->GetComponent<Network>()->SetOwner(NetworkRole::Server);

    Entity *cloud_2 = new Entity("cloud_2", Stationary);
    cloud_2->AddComponent<Render>();
    cloud_2->AddComponent<Transform>();
    cloud_2->AddComponent<Collision>();
    cloud_2->AddComponent<Network>();
    cloud_2->AddComponent<Physics>();
    cloud_2->AddComponent<Handler>();

    cloud_2->GetComponent<Physics>()->SetEngineTimeline(engine_ptr->GetBaseTimeline());

    cloud_2->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    cloud_2->GetComponent<Render>()->SetTexture("assets/cloud_2.png");
    cloud_2->GetComponent<Transform>()->SetPosition(
        Position{float(window_size.width) / 2 + 300, (TILE_SIZE * 1.5)});
    cloud_2->GetComponent<Transform>()->SetSize(Size{216, 139});
    cloud_2->GetComponent<Collision>()->SetAffectedByCollision(false);
    cloud_2->GetComponent<Network>()->SetOwner(NetworkRole::Server);

    return std::vector({cloud_1, cloud_2});
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

        enemy->GetComponent<Physics>()->SetEngineTimeline(engine_ptr->GetBaseTimeline());

        enemy->GetComponent<Transform>()->SetPosition(Position{
            float(window_size.width + TILE_SIZE), float(window_size.height - (TILE_SIZE * 2))});
        enemy->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
        enemy->GetComponent<Physics>()->SetVelocity(Velocity{0, 0});
        enemy->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        enemy->GetComponent<Render>()->SetTexture(enemy_texture);
        enemy->GetComponent<Collision>()->SetAffectedByCollision(false);
        enemy->GetComponent<Network>()->SetOwner(NetworkRole::Server);
        enemies.push_back(enemy);
        enemy_index += 1;
    }
    return enemies;
}

std::vector<Entity *> CreateEntities() {
    Entity *alien = CreateAlien();
    Entity *platform = CreatePlatform();
    std::vector<Entity *> clouds = CreateClouds();
    std::vector<Entity *> ground = CreateGround();
    std::vector<Entity *> enemies = CreateEnemies();

    std::vector<Entity *> entities = std::vector({alien, platform});
    for (Entity *cloud : clouds) {
        entities.push_back(cloud);
    }
    for (Entity *ground_tile : ground) {
        entities.push_back(ground_tile);
    }
    for (Entity *enemy : enemies) {
        entities.push_back(enemy);
    }

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
    std::string title = "CSC581 HW3 Joshua's Game";

    Engine engine;
    engine_ptr = &engine;
    if (!SetEngineCLIOptions(&engine, argc, args)) {
        return 1;
    }
    network_info = engine.GetNetworkInfo();

    Color background_color = Color{52, 153, 219, 255};
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
    engine.SetEntities(entities);
    engine.SetPlayerTextures(5);
    engine.SetCallback(Update);

    engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    DestroyEntities(entities);
    return 0;
}