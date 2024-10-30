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
            if (collider->GetName().find("house") == 0) {
                Log(LogLevel::Info, "");
                Log(LogLevel::Info, "You made it home!");
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

    if (platform_right_edge >= 2600 && platform->GetComponent<Physics>()->GetVelocity().x > 0) {
        platform->GetComponent<Physics>()->SetVelocity(
            {-std::abs(platform->GetComponent<Physics>()->GetVelocity().x),
             platform->GetComponent<Physics>()->GetVelocity().y});
    }
    if (platform_left_edge <= 880 && platform->GetComponent<Physics>()->GetVelocity().x < 0) {
        platform->GetComponent<Physics>()->SetVelocity(
            {std::abs(platform->GetComponent<Physics>()->GetVelocity().x),
             platform->GetComponent<Physics>()->GetVelocity().y});
    }
}

void UpdateEnemy(Entity *enemy) {
    float enemy_bottom_edge = enemy->GetComponent<Transform>()->GetPosition().y +
                              float(enemy->GetComponent<Transform>()->GetSize().height);
    float enemy_top_edge = enemy->GetComponent<Transform>()->GetPosition().y;

    if (enemy_bottom_edge >= float(window_size.height - TILE_SIZE) &&
        enemy->GetComponent<Physics>()->GetVelocity().y > 0) {
        enemy->GetComponent<Physics>()->SetVelocity({
            enemy->GetComponent<Physics>()->GetVelocity().x,
            -std::abs(enemy->GetComponent<Physics>()->GetVelocity().y),
        });
    }
    if (enemy_top_edge <= float(window_size.height - (TILE_SIZE * 5)) &&
        enemy->GetComponent<Physics>()->GetVelocity().y < 0) {
        enemy->GetComponent<Physics>()->SetVelocity({
            enemy->GetComponent<Physics>()->GetVelocity().x,
            std::abs(enemy->GetComponent<Physics>()->GetVelocity().y),
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
    platform->GetComponent<Transform>()->SetPosition(Position{20, TILE_SIZE * 5});
    platform->GetComponent<Transform>()->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->GetComponent<Physics>()->SetVelocity(Velocity{40, 0});
    platform->GetComponent<Handler>()->SetCallback(UpdatePlatform);
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
        enemy->GetComponent<Handler>()->SetCallback(UpdateEnemy);
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

void CreateSideBoundaries(Engine *engine) {
    engine->AddSideBoundary(Position{500, -float(window_size.height)},
                            Size{10, window_size.height * 3});
    engine->AddSideBoundary(Position{1410, -float(window_size.height)},
                            Size{10, window_size.height * 3});
}

void CreateSpawnPoints(Engine *engine) {
    engine->AddSpawnPoint(Position{530, float(window_size.height - (TILE_SIZE * 3))}, Size{10, 10});
    engine->AddSpawnPoint(Position{530, 40}, Size{10, 10});
    engine->AddSpawnPoint(Position{1290, 40}, Size{10, 10});
    engine->AddSpawnPoint(Position{1290, float(window_size.height - (TILE_SIZE * 3))},
                          Size{10, 10});
}

void CreateDeathZones(Engine *engine) {
    engine->AddDeathZone(Position{-10, -float(window_size.height)},
                         Size{10, window_size.height * 3});
    engine->AddDeathZone(Position{3800, -float(window_size.height)},
                         Size{10, window_size.height * 3});
    engine->AddDeathZone(Position{-float(window_size.width), -float(TILE_SIZE * 3)},
                         Size{window_size.width * 4, 10});
    engine->AddDeathZone(Position{-float(window_size.width), float(window_size.height - 10)},
                         Size{window_size.width * 4, 10});
}

void DestroyEntities(std::vector<Entity *> entities) {
    for (Entity *entity : entities) {
        delete entity;
    }
}

void SetupInputs(Engine *engine) {
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
    // toggle the visibility of hidden zones
    app->key_map->key_Z.OnPress = [engine]() { engine->ToggleShowZoneBorders(); };
}

int main(int argc, char *args[]) {
    std::string title = "CSC581 HW4 Joshua's Game";

    Engine engine;
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

    SetupInputs(&engine);
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