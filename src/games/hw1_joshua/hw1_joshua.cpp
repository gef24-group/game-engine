#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <random>
#include <string>

const int TILE_SIZE = 100;

// All textures sourced from:
// https://kenney.nl/assets/background-elements-redux
// https://kenney.nl/assets/platformer-pack-redux

// Game update code
void Update(std::vector<GameObject *> *game_objects) {
    // Generate a random number
    std::random_device random_device;
    std::mt19937 eng(random_device());
    std::uniform_int_distribution<> distr(1, 10);
    int random_number = distr(eng);

    float max_ground_x = 0;
    GameObject *max_ground = nullptr;
    for (GameObject *game_object : *game_objects) {
        if (game_object->GetName() == "ground") {
            if (game_object->GetPosition().x > max_ground_x) {
                max_ground = game_object;
                max_ground_x = game_object->GetPosition().x;
            }
        }
    }

    bool ground_repositioned = false;
    for (GameObject *game_object : *game_objects) {
        if (game_object->GetName() == "ground") {
            int right_edge = static_cast<int>(std::round(game_object->GetPosition().x)) +
                             game_object->GetSize().width;
            if (right_edge <= 0) {
                ground_repositioned = true;
                game_object->SetPosition(
                    Position{max_ground_x + TILE_SIZE, game_object->GetPosition().y});
            }
        }
    }

    std::vector<const char *> enemies = std::vector(
        {"assets/ladybug.png", "assets/mouse.png", "assets/worm.png", "assets/bee.png"});
    std::uniform_int_distribution<> enemy_distr(0, int(enemies.size() - 1));
    size_t random_enemy = enemy_distr(eng);

    if (random_number == 5 && max_ground != nullptr && ground_repositioned) {
        GameObject *enemy = new GameObject("enemy", Moving);
        enemy->SetPosition(Position{max_ground_x, float(max_ground->GetPosition().y - TILE_SIZE)});
        enemy->SetVelocity(Velocity{-50, 0});
        enemy->SetSize(Size{TILE_SIZE, TILE_SIZE});
        enemy->SetColor(Color{0, 0, 0, 0});
        enemy->SetReduceVelocityOnCollision(false);
        enemy->SetTexture(enemies[random_enemy]);
        game_objects->push_back(enemy);
    }

    float min_enemy_x = float(GetWindowSize().width * 2.0);
    GameObject *min_enemy = nullptr;
    for (GameObject *game_object : *game_objects) {
        if (game_object->GetName() == "enemy") {
            if (game_object->GetPosition().x < min_enemy_x) {
                min_enemy = game_object;
                min_enemy_x = game_object->GetPosition().x;
            }
        }
    }
    if (min_enemy_x + TILE_SIZE <= 0 && min_enemy != nullptr) {
        game_objects->erase(
            std::remove_if(game_objects->begin(), game_objects->end(),
                           [min_enemy](GameObject *obj) { return obj == min_enemy; }),
            game_objects->end());
        delete min_enemy;
    }

    if (max_ground != nullptr) {
        GameObject *platform = GetObjectByName("platform", *game_objects);
        float platform_right_edge = platform->GetPosition().x + float(platform->GetSize().width);
        float platform_left_edge = platform->GetPosition().x;

        if (platform_right_edge >= max_ground->GetPosition().x && platform->GetVelocity().x > 0) {
            platform->SetVelocity({-40, platform->GetVelocity().y});
        }

        if (platform_left_edge <= 0 && platform->GetVelocity().x < 0) {
            platform->SetVelocity({40, platform->GetVelocity().y});
        }
    }
}

void UpdateAlien(GameObject *alien) {
    if (app->key_map->key_up.pressed || app->key_map->key_W.pressed) {
        alien->SetVelocity({alien->GetVelocity().x, -60});
    }

    if (app->key_map->key_down.pressed || app->key_map->key_S.pressed) {
        alien->SetVelocity({alien->GetVelocity().x, 60});
    }

    if (app->key_map->key_left.pressed || app->key_map->key_A.pressed) {
        alien->SetVelocity({-60, alien->GetVelocity().y});
    } else if (app->key_map->key_right.pressed || app->key_map->key_D.pressed) {
        alien->SetVelocity({60, alien->GetVelocity().y});
    } else {
        alien->SetVelocity({0, alien->GetVelocity().y});
    }

    if (alien->GetColliders().size() > 0) {
        for (GameObject *game_object : alien->GetColliders()) {
            if (game_object->GetName() == "enemy") {
                Log(LogLevel::Info, "");
                Log(LogLevel::Info, "You lost :(");
                Log(LogLevel::Info, "");
                app->quit = true;
                break;
            }
        }
    }
}

void GenerateGround(std::vector<GameObject *> *ground) {
    for (int i = 0; i < (GetWindowSize().width / TILE_SIZE) + 2; i++) {
        GameObject *ground_tile = new GameObject("ground", Moving);
        ground_tile->SetColor(Color{0, 0, 0, 0});
        ground_tile->SetPosition(
            Position{float(i * TILE_SIZE), float(GetWindowSize().height - TILE_SIZE)});
        ground_tile->SetSize(Size{TILE_SIZE, TILE_SIZE});
        ground_tile->SetVelocity(Velocity{-50, 0});
        ground_tile->SetReduceVelocityOnCollision(false);
        ground_tile->SetTexture("assets/ground.png");

        ground->push_back(ground_tile);
    }
}

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW1 Joshua's Game";

    // Initializing the Game Engine
    GameEngine game_engine;
    Color background_color = Color{52, 153, 219, 255};
    game_engine.SetBackgroundColor(background_color);
    if (!game_engine.Init(game_title.c_str())) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    GameObject alien("alien", Controllable);
    alien.SetColor(Color{0, 0, 0, 0});
    alien.SetPosition(Position{20, float(GetWindowSize().height - (TILE_SIZE * 5))});
    alien.SetSize(Size{TILE_SIZE, TILE_SIZE});
    alien.SetAcceleration(Acceleration{0, 15});
    alien.SetVelocity(Velocity{0, 0});
    alien.SetRestitution(0.5);
    alien.SetTexture("assets/alien.png");
    alien.SetCallback(UpdateAlien);

    std::vector<GameObject *> ground;
    GenerateGround(&ground);

    GameObject cloud_1("cloud", Stationary);
    cloud_1.SetColor(Color{0, 0, 0, 0});
    cloud_1.SetTexture("assets/cloud_1.png");
    cloud_1.SetPosition(Position{float(GetWindowSize().width) / 2 - 500, TILE_SIZE});
    cloud_1.SetSize(Size{203, 121});

    GameObject cloud_2("cloud", Stationary);
    cloud_2.SetColor(Color{0, 0, 0, 0});
    cloud_2.SetTexture("assets/cloud_2.png");
    cloud_2.SetPosition(Position{float(GetWindowSize().width) / 2 + 300, TILE_SIZE});
    cloud_2.SetSize(Size{216, 139});

    GameObject platform("platform", Moving);
    platform.SetColor(Color{0, 0, 0, 0});
    platform.SetPosition(Position{20, TILE_SIZE * 4});
    platform.SetSize(Size{TILE_SIZE * 3, TILE_SIZE / 2});
    platform.SetVelocity(Velocity{40, 0});
    platform.SetReduceVelocityOnCollision(false);
    platform.SetTexture("assets/stone.png");

    std::vector<GameObject *> objects = std::vector({&alien, &cloud_1, &cloud_2, &platform});
    for (GameObject *ground_tile : ground) {
        objects.push_back(ground_tile);
    }
    game_engine.AddObjects(objects);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    return 0;
}