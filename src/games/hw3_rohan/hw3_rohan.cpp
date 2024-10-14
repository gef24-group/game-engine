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
#include <atomic>
#include <string>
#include <vector>

NetworkInfo network_info;
Size window_size;

void Update(std::vector<Entity *> *entities) {}

void AssignOperationsToKeys(Engine *game_engine) {
    // toggle the visibility of the spawn points, death zone and side boundaries
    app->key_map->key_Z.OnPress = [game_engine]() { game_engine->ToggleShowZoneBorders(); };
    // toggle constant and proportional scaling
    app->key_map->key_X.OnPress = []() {
        app->window.proportional_scaling = !app->window.proportional_scaling;
    };
    // toggle pause or unpause
    app->key_map->key_P.OnPress = [game_engine]() { game_engine->BaseTimelineTogglePause(); };
    // slow down the timeline
    app->key_map->key_comma.OnPress = [game_engine]() {
        game_engine->BaseTimelineChangeTic(std::min(game_engine->BaseTimelineGetTic() * 2.0, 2.0));
    };
    // speed up the timeline
    app->key_map->key_period.OnPress = [game_engine]() {
        game_engine->BaseTimelineChangeTic(std::max(game_engine->BaseTimelineGetTic() / 2.0, 0.5));
    };
}

void UpdatePlayer(Entity *player) {
    bool player_moved = false;

    if (app->key_map->key_right.pressed.load()) {
        player_moved = true;
        player->GetComponent<Physics>()->SetVelocity(
            {20, player->GetComponent<Physics>()->GetVelocity().y}); // Move left
    } else if (app->key_map->key_left.pressed.load()) {
        player_moved = true;
        player->GetComponent<Physics>()->SetVelocity(
            {-20, player->GetComponent<Physics>()->GetVelocity().y}); // Move right
    }

    if (app->key_map->key_up.pressed.load()) {
        player_moved = true;
        player->GetComponent<Physics>()->SetVelocity(
            {player->GetComponent<Physics>()->GetVelocity().x, -20});
    }

    if (!player_moved) {
        player->GetComponent<Physics>()->SetVelocity(
            {0, player->GetComponent<Physics>()->GetVelocity().y});
    }
}

std::vector<Entity *> CreateGround() {
    std::vector<Entity *> ground_blocks;
    Color ground_color = {50, 200, 100, 255};

    Entity *ground_block1 = new Entity("ground_1", EntityCategory::Stationary);
    ground_block1->AddComponent<Transform>();
    ground_block1->AddComponent<Collision>();
    ground_block1->AddComponent<Render>();
    ground_block1->GetComponent<Transform>()->SetPosition({-500, float(window_size.height) - 200});
    ground_block1->GetComponent<Transform>()->SetSize({1300, 200});
    ground_block1->GetComponent<Render>()->SetColor(ground_color);
    ground_blocks.push_back(ground_block1);

    Entity *ground_block2 = new Entity("ground_2", EntityCategory::Stationary);
    ground_block2->AddComponent<Transform>();
    ground_block2->AddComponent<Collision>();
    ground_block2->AddComponent<Render>();
    ground_block2->GetComponent<Transform>()->SetPosition({800, float(window_size.height) - 400});
    ground_block2->GetComponent<Transform>()->SetSize({200, 400});
    ground_block2->GetComponent<Render>()->SetColor(ground_color);
    ground_blocks.push_back(ground_block2);

    Entity *ground_block3 = new Entity("ground_3", EntityCategory::Stationary);
    ground_block3->AddComponent<Transform>();
    ground_block3->AddComponent<Collision>();
    ground_block3->AddComponent<Render>();
    ground_block3->GetComponent<Transform>()->SetPosition({1000, float(window_size.height) - 600});
    ground_block3->GetComponent<Transform>()->SetSize({200, 600});
    ground_block3->GetComponent<Render>()->SetColor(ground_color);
    ground_blocks.push_back(ground_block3);

    Entity *ground_block4 = new Entity("ground_4", EntityCategory::Stationary);
    ground_block4->AddComponent<Transform>();
    ground_block4->AddComponent<Collision>();
    ground_block4->AddComponent<Render>();
    ground_block4->GetComponent<Transform>()->SetPosition({1200, float(window_size.height) - 800});
    ground_block4->GetComponent<Transform>()->SetSize({200, 800});
    ground_block4->GetComponent<Render>()->SetColor(ground_color);
    ground_blocks.push_back(ground_block4);

    Entity *ground_block5 = new Entity("ground_5", EntityCategory::Stationary);
    ground_block5->AddComponent<Transform>();
    ground_block5->AddComponent<Collision>();
    ground_block5->AddComponent<Render>();
    ground_block5->GetComponent<Transform>()->SetPosition({1800, float(window_size.height) - 800});
    ground_block5->GetComponent<Transform>()->SetSize({200, 800});
    ground_block5->GetComponent<Render>()->SetColor(ground_color);
    ground_blocks.push_back(ground_block5);

    Entity *ground_block6 = new Entity("ground_6", EntityCategory::Stationary);
    ground_block6->AddComponent<Transform>();
    ground_block6->AddComponent<Collision>();
    ground_block6->AddComponent<Render>();
    ground_block6->GetComponent<Transform>()->SetPosition({2000, float(window_size.height) - 600});
    ground_block6->GetComponent<Transform>()->SetSize({200, 600});
    ground_block6->GetComponent<Render>()->SetColor(ground_color);
    ground_blocks.push_back(ground_block6);

    Entity *ground_block7 = new Entity("ground_7", EntityCategory::Stationary);
    ground_block7->AddComponent<Transform>();
    ground_block7->AddComponent<Collision>();
    ground_block7->AddComponent<Render>();
    ground_block7->GetComponent<Transform>()->SetPosition({2200, float(window_size.height) - 400});
    ground_block7->GetComponent<Transform>()->SetSize({200, 400});
    ground_block7->GetComponent<Render>()->SetColor(ground_color);
    ground_blocks.push_back(ground_block7);

    Entity *ground_block8 = new Entity("ground_8", EntityCategory::Stationary);
    ground_block8->AddComponent<Transform>();
    ground_block8->AddComponent<Collision>();
    ground_block8->AddComponent<Render>();
    ground_block8->GetComponent<Transform>()->SetPosition({2400, float(window_size.height) - 200});
    ground_block8->GetComponent<Transform>()->SetSize({1300, 200});
    ground_block8->GetComponent<Render>()->SetColor(ground_color);
    ground_blocks.push_back(ground_block8);

    return ground_blocks;
}

std::vector<Entity *> CreatePlatforms() {
    std::vector<Entity *> platforms;
    // ADD CODE HERE
    return platforms;
}

Entity *CreatePlayer() {
    Entity *player = new Entity("player", Controllable);
    player->AddComponent<Transform>();
    player->AddComponent<Physics>();
    player->AddComponent<Render>();
    player->AddComponent<Collision>();
    player->AddComponent<Network>();
    player->AddComponent<Handler>();

    player->GetComponent<Transform>()->SetPosition({300, 300});
    player->GetComponent<Transform>()->SetSize({50, 100});

    player->GetComponent<Physics>()->SetAcceleration({0, 10});
    player->GetComponent<Render>()->SetTextureTemplate("assets/player_{}.png");
    player->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    player->GetComponent<Handler>()->SetCallback(UpdatePlayer);

    return player;
}

void CreateSideBoundaries(Engine *game_engine) {
    game_engine->AddSideBoundary(Position{100, -1000}, Size{20, 3000});
    game_engine->AddSideBoundary(Position{float(window_size.width - 400), -1000}, Size{20, 3000});
    game_engine->AddSideBoundary(Position{-600, float(window_size.height - 1000)}, Size{4000, 20});
    game_engine->AddSideBoundary(Position{-600, float(window_size.height - 100)}, Size{4000, 20});
}

void CreateDeathZones(Engine *game_engine) {}

void CreateSpawnPoints(Engine *game_engine) {}

// std::vector<Entity *> CreateFire() {}

std::vector<Entity *> CreateEntities(Engine *game_engine) {
    std::vector<Entity *> entities;

    Entity *player = CreatePlayer();
    std::vector<Entity *> ground = CreateGround();
    CreateSideBoundaries(game_engine);

    entities.push_back(player);
    entities.insert(entities.end(), ground.begin(), ground.end());
    return entities;
}

void AddObjectsToEngine(std::vector<Entity *> entities, Engine *game_engine) {
    for (Entity *entity : entities) {
        game_engine->AddEntity(entity);
    }
}

void DestroyEntities(std::vector<Entity *> entities) {
    for (Entity *entity : entities) {
        delete entity;
    }
}

int main(int argc, char *args[]) {
    std::string game_title = "Rohan's CSC581 HW3 Game: Platformer";
    int max_player_count = 100, texture_count = 1;

    // Initializing the Game Engine
    Engine game_engine;
    if (!SetEngineCLIOptions(&game_engine, argc, args)) {
        return 1;
    }

    if (!game_engine.Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    AssignOperationsToKeys(&game_engine);
    game_engine.SetPlayerTextures(texture_count);
    game_engine.SetMaxPlayers(max_player_count);
    game_engine.SetShowPlayerBorder(true);

    network_info = game_engine.GetNetworkInfo();
    if (network_info.id > 4) {
        Log(LogLevel::Error, "More than 4 players spotted: EXITING THE GAME. Player ID: %d",
            network_info.id);
        exit(0);
    }

    Color background_color = Color{165, 200, 255, 255};
    game_engine.SetBackgroundColor(background_color);
    game_engine.SetTitle(game_title);

    window_size = GetWindowSize();

    std::vector<Entity *> entities = CreateEntities(&game_engine);
    AddObjectsToEngine(entities, &game_engine);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");

    // Add Game Cleanup code (deallocating pointers)
    DestroyEntities(entities);
    return 0;
}