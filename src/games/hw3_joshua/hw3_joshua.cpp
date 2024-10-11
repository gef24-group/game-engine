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
#include <string>

const int TILE_SIZE = 100;
Size window_size;
NetworkInfo network_info;
std::vector<const char *> enemy_textures =
    std::vector({"assets/ladybug.png", "assets/mouse.png", "assets/worm.png", "assets/bee.png"});

void Update(std::vector<Entity *> *entities) {}

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
        for (Entity *collider : alien->GetComponent<Collision>()->GetColliders()) {
            if (collider->GetName().find("enemy") == 0) {
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

std::vector<Entity *> CreateSpawnPoints() {
    std::vector<Entity *> spawn_points;

    return spawn_points;
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
    alien->GetComponent<Handler>()->SetCallback(UpdateAlien);
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
    platform->GetComponent<Transform>()->SetPosition(Position{20, TILE_SIZE * 5.5});
    platform->GetComponent<Transform>()->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->GetComponent<Physics>()->SetVelocity(Velocity{40, 0});
    platform->GetComponent<Handler>()->SetCallback(UpdatePlatform);
    platform->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    return platform;
}

std::vector<Entity *> CreateGround() {
    std::vector<Entity *> ground;
    for (int i = 0; i < (window_size.width / TILE_SIZE) * 3; i++) {
        Entity *ground_tile = new Entity("ground_" + std::to_string(i), EntityCategory::Moving);
        ground_tile->AddComponent<Render>();
        ground_tile->AddComponent<Transform>();
        ground_tile->AddComponent<Network>();
        ground_tile->AddComponent<Handler>();

        ground_tile->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        ground_tile->GetComponent<Transform>()->SetPosition(
            Position{float(i * TILE_SIZE), float(window_size.height - TILE_SIZE)});
        ground_tile->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
        ground_tile->GetComponent<Render>()->SetTexture("assets/ground.png");
        ground_tile->GetComponent<Network>()->SetOwner(NetworkRole::Server);
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
    cloud_1->GetComponent<Transform>()->SetPosition(
        Position{float(window_size.width) / 2 - 500, (TILE_SIZE * 1.5)});
    cloud_1->GetComponent<Transform>()->SetSize(Size{203, 121});
    cloud_1->GetComponent<Network>()->SetOwner(NetworkRole::Server);

    Entity *cloud_2 = new Entity("cloud_2", EntityCategory::Stationary);
    cloud_2->AddComponent<Render>();
    cloud_2->AddComponent<Transform>();
    cloud_2->AddComponent<Network>();

    cloud_2->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    cloud_2->GetComponent<Render>()->SetTexture("assets/cloud_2.png");
    cloud_2->GetComponent<Transform>()->SetPosition(
        Position{float(window_size.width) / 2 + 300, (TILE_SIZE * 1.5)});
    cloud_2->GetComponent<Transform>()->SetSize(Size{216, 139});
    cloud_2->GetComponent<Network>()->SetOwner(NetworkRole::Server);

    return std::vector({cloud_1, cloud_2});
}

std::vector<Entity *> CreateEnemies() {
    std::vector<Entity *> enemies;
    int enemy_index = 0;
    while (enemy_index < (window_size.width / TILE_SIZE) * 3) {
        Entity *enemy = new Entity("enemy_" + std::to_string(enemy_index), EntityCategory::Moving);
        enemy->AddComponent<Transform>();
        enemy->AddComponent<Render>();
        enemy->AddComponent<Network>();

        enemy->GetComponent<Transform>()->SetPosition(
            Position{float(enemy_index * TILE_SIZE), float(window_size.height - (TILE_SIZE * 2))});
        enemy->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
        enemy->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
        enemy->GetComponent<Render>()->SetTexture(
            enemy_textures[enemy_index % enemy_textures.size()]);
        enemy->GetComponent<Network>()->SetOwner(NetworkRole::Server);
        enemies.push_back(enemy);

        enemy_index += 5;
    }
    return std::vector<Entity *>();
    // return enemies;
}

std::vector<Entity *> CreateEntities() {
    Entity *alien = CreateAlien();
    Entity *platform = CreatePlatform();
    std::vector<Entity *> ground = CreateGround();
    std::vector<Entity *> clouds = CreateClouds();
    std::vector<Entity *> enemies = CreateEnemies();

    std::vector<Entity *> entities = std::vector({alien, platform});
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

void CreateSideBoundaries(Engine *engine) {
    engine->AddSideBoundary(Position{float(window_size.width) - 300, 0},
                            Size{10, window_size.height - TILE_SIZE});
    engine->AddSideBoundary(Position{200, 0}, Size{10, window_size.height - TILE_SIZE});
}

void CreateSpawnPoints(Engine *engine) {
    engine->AddSpawnPoint(Position{320, float(window_size.height - (TILE_SIZE * 5))}, Size{0, 0});
}

void CreateDeathZones(Engine *engine) {
    engine->AddDeathZone(Position{-TILE_SIZE, 0}, Size{TILE_SIZE, window_size.height});
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
    if (!SetEngineCLIOptions(&engine, argc, args)) {
        return 1;
    }
    network_info = engine.GetNetworkInfo();

    Color background_color = Color{52, 153, 219, 255};
    engine.SetBackgroundColor(background_color);
    engine.SetTitle(title);
    engine.SetShowPlayerBorder(true);
    engine.SetShowZoneBorders(true);

    if (!engine.Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    SetupTimelineInputs(&engine);
    network_info = engine.GetNetworkInfo();
    window_size = GetWindowSize();

    CreateSideBoundaries(&engine);
    CreateSpawnPoints(&engine);
    CreateDeathZones(&engine);

    std::vector<Entity *> entities = CreateEntities();
    for (Entity *entity : entities) {
        engine.AddEntity(entity);
    }

    engine.SetPlayerTextures(5);
    engine.SetCallback(Update);

    engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    DestroyEntities(entities);
    return 0;
}