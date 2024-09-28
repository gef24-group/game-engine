#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <random>
#include <string>

const int TILE_SIZE = 100;
Size window_size;
NetworkInfo network_info;
std::vector<const char *> enemy_textures =
    std::vector({"assets/ladybug.png", "assets/mouse.png", "assets/worm.png", "assets/bee.png"});

void Update(std::vector<GameObject *> *game_objects) {
    std::random_device random_device;
    std::mt19937 eng(random_device());
    std::uniform_int_distribution<> include_dist(0, 25);
    std::uniform_int_distribution<> enemy_dist(0, int(enemy_textures.size() - 1));

    bool include_enemy = include_dist(eng) == 12;
    size_t random_enemy = enemy_dist(eng);

    float max_ground_x = 0;
    GameObject *max_ground = nullptr;
    for (GameObject *game_object : *game_objects) {
        if (game_object->GetName().find("ground") == 0) {
            if (game_object->GetPosition().x > max_ground_x) {
                max_ground = game_object;
                max_ground_x = game_object->GetPosition().x;
            }
        }
    }

    bool ground_repositioned = false;
    for (GameObject *game_object : *game_objects) {
        if (game_object->GetName().find("ground") == 0) {
            int right_edge = static_cast<int>(std::round(game_object->GetPosition().x)) +
                             game_object->GetSize().width;
            if (right_edge <= 0) {
                ground_repositioned = true;
                game_object->SetPosition(
                    Position{max_ground_x + TILE_SIZE, float(window_size.height - TILE_SIZE)});
            }
        }
    }

    int enemy_index = 0;
    if (include_enemy && max_ground != nullptr && ground_repositioned) {
        for (GameObject *game_object : *game_objects) {
            if (game_object->GetName().find("enemy") == 0) {
                if (enemy_index == random_enemy) {
                    game_object->SetVelocity(Velocity{-50, 0});
                }
                enemy_index += 1;
            }
        }
    }

    float min_enemy_x = float(window_size.width * 2.0);
    GameObject *min_enemy = nullptr;
    for (GameObject *game_object : *game_objects) {
        if (game_object->GetName().find("enemy") == 0) {
            if (game_object->GetPosition().x < min_enemy_x) {
                min_enemy = game_object;
                min_enemy_x = game_object->GetPosition().x;
            }
        }
    }
    if (min_enemy_x + TILE_SIZE <= 0 && min_enemy != nullptr) {
        min_enemy->SetPosition(Position{float(window_size.width + TILE_SIZE),
                                        float(window_size.height - (TILE_SIZE * 2))});
        min_enemy->SetVelocity(Velocity{0, 0});
    }
}

void UpdateAlien(GameObject *alien) {
    if (app->key_map->key_up.pressed.load() || app->key_map->key_W.pressed.load()) {
        alien->SetVelocity({alien->GetVelocity().x, -60});
    }

    if (app->key_map->key_down.pressed.load() || app->key_map->key_S.pressed.load()) {
        alien->SetVelocity({alien->GetVelocity().x, 60});
    }

    if (app->key_map->key_left.pressed.load() || app->key_map->key_A.pressed.load()) {
        alien->SetVelocity({-60, alien->GetVelocity().y});
    } else if (app->key_map->key_right.pressed.load() || app->key_map->key_D.pressed.load()) {
        alien->SetVelocity({60, alien->GetVelocity().y});
    } else {
        alien->SetVelocity({0, alien->GetVelocity().y});
    }

    if (alien->GetName() == "alien_" + std::to_string(network_info.id) &&
        alien->GetColliders().size() > 0) {
        for (GameObject *game_object : alien->GetColliders()) {
            if (game_object->GetName().find("enemy") == 0) {
                Log(LogLevel::Info, "");
                Log(LogLevel::Info, "You lost :(");
                Log(LogLevel::Info, "");
                app->quit.store(true);
                break;
            }
        }
    }
}

void UpdatePlatform(GameObject *platform) {
    float platform_right_edge = platform->GetPosition().x + float(platform->GetSize().width);
    float platform_left_edge = platform->GetPosition().x;

    if (platform_right_edge >= float(window_size.width) && platform->GetVelocity().x > 0) {
        platform->SetVelocity({-std::abs(platform->GetVelocity().x), platform->GetVelocity().y});
    }
    if (platform_left_edge <= 0 && platform->GetVelocity().x < 0) {
        platform->SetVelocity({std::abs(platform->GetVelocity().x), platform->GetVelocity().y});
    }
}

GameObject *CreateAlien() {
    GameObject *alien = new GameObject("alien", Controllable);
    alien->SetColor(Color{0, 0, 0, 0});
    alien->SetPosition(Position{20, float(window_size.height - (TILE_SIZE * 5))});
    alien->SetSize(Size{TILE_SIZE, TILE_SIZE});
    alien->SetAcceleration(Acceleration{0, 15});
    alien->SetVelocity(Velocity{0, 0});
    alien->SetRestitution(0.5);
    alien->SetTextureTemplate("assets/alien_{}.png");
    alien->SetCallback(UpdateAlien);
    alien->SetOwner(NetworkRole::Client);
    return alien;
}

GameObject *CreatePlatform() {
    GameObject *platform = new GameObject("platform", Moving);
    platform->SetColor(Color{0, 0, 0, 0});
    platform->SetPosition(Position{20, TILE_SIZE * 5.5});
    platform->SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform->SetVelocity(Velocity{40, 0});
    platform->SetAffectedByCollision(false);
    platform->SetTexture("assets/stone.png");
    platform->SetCallback(UpdatePlatform);
    platform->SetOwner(NetworkRole::Server);
    return platform;
}

std::vector<GameObject *> CreateGround() {
    std::vector<GameObject *> ground;
    for (int i = 0; i < (window_size.width / TILE_SIZE) + 2; i++) {
        GameObject *ground_tile = new GameObject("ground_" + std::to_string(i), Moving);
        ground_tile->SetColor(Color{0, 0, 0, 0});
        ground_tile->SetPosition(
            Position{float(i * TILE_SIZE), float(window_size.height - TILE_SIZE)});
        ground_tile->SetSize(Size{TILE_SIZE, TILE_SIZE});
        ground_tile->SetVelocity(Velocity{-50, 0});
        ground_tile->SetAffectedByCollision(false);
        ground_tile->SetTexture("assets/ground.png");
        ground_tile->SetOwner(NetworkRole::Server);
        ground.push_back(ground_tile);
    }
    return ground;
}

std::vector<GameObject *> CreateClouds() {
    GameObject *cloud_1 = new GameObject("cloud_1", Stationary);
    cloud_1->SetColor(Color{0, 0, 0, 0});
    cloud_1->SetTexture("assets/cloud_1.png");
    cloud_1->SetPosition(Position{float(window_size.width) / 2 - 500, (TILE_SIZE * 1.5)});
    cloud_1->SetSize(Size{203, 121});
    cloud_1->SetAffectedByCollision(false);
    cloud_1->SetOwner(NetworkRole::Server);

    GameObject *cloud_2 = new GameObject("cloud_2", Stationary);
    cloud_2->SetColor(Color{0, 0, 0, 0});
    cloud_2->SetTexture("assets/cloud_2.png");
    cloud_2->SetPosition(Position{float(window_size.width) / 2 + 300, (TILE_SIZE * 1.5)});
    cloud_2->SetSize(Size{216, 139});
    cloud_2->SetAffectedByCollision(false);
    cloud_2->SetOwner(NetworkRole::Server);

    return std::vector({cloud_1, cloud_2});
}

std::vector<GameObject *> CreateEnemies() {
    std::vector<GameObject *> enemies;
    int enemy_index = 0;
    for (const char *enemy_texture : enemy_textures) {
        GameObject *enemy = new GameObject("enemy_" + std::to_string(enemy_index), Moving);
        enemy->SetPosition(Position{float(window_size.width + TILE_SIZE),
                                    float(window_size.height - (TILE_SIZE * 2))});
        enemy->SetVelocity(Velocity{0, 0});
        enemy->SetSize(Size{TILE_SIZE, TILE_SIZE});
        enemy->SetColor(Color{0, 0, 0, 0});
        enemy->SetAffectedByCollision(false);
        enemy->SetTexture(enemy_texture);
        enemy->SetOwner(NetworkRole::Server);
        enemies.push_back(enemy);
        enemy_index += 1;
    }
    return enemies;
}

std::vector<GameObject *> CreateGameObjects() {
    GameObject *alien = CreateAlien();
    GameObject *platform = CreatePlatform();
    std::vector<GameObject *> clouds = CreateClouds();
    std::vector<GameObject *> ground = CreateGround();
    std::vector<GameObject *> enemies = CreateEnemies();

    std::vector<GameObject *> game_objects = std::vector({alien, platform});
    for (GameObject *cloud : clouds) {
        game_objects.push_back(cloud);
    }
    for (GameObject *ground_tile : ground) {
        game_objects.push_back(ground_tile);
    }
    for (GameObject *enemy : enemies) {
        game_objects.push_back(enemy);
    }

    for (GameObject *game_object : game_objects) {
        if (network_info.mode == NetworkMode::PeerToPeer) {
            if (game_object->GetOwner() == NetworkRole::Server) {
                game_object->SetOwner(NetworkRole::Host);
            }
            if (game_object->GetOwner() == NetworkRole::Client) {
                game_object->SetOwner(NetworkRole::Peer);
            }
        }
    }

    return game_objects;
}

void DestroyObjects(std::vector<GameObject *> game_objects) {
    for (GameObject *object : game_objects) {
        delete object;
    }
}

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW2 Joshua's Game";

    GameEngine game_engine;
    if (!SetEngineCLIOptions(&game_engine, argc, args)) {
        return 1;
    }
    network_info = game_engine.GetNetworkInfo();

    Color background_color = Color{52, 153, 219, 255};
    game_engine.SetBackgroundColor(background_color);
    game_engine.SetGameTitle(game_title);
    game_engine.SetShowPlayerBorder(true);

    if (!game_engine.Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }
    network_info = game_engine.GetNetworkInfo();
    window_size = GetWindowSize();

    std::vector<GameObject *> game_objects = CreateGameObjects();
    game_engine.AddObjects(game_objects);
    game_engine.SetPlayerTextures(5);
    game_engine.SetCallback(Update);

    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    DestroyObjects(game_objects);
    return 0;
}