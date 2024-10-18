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
            {30, player->GetComponent<Physics>()->GetVelocity().y}); // Move left
    } else if (app->key_map->key_left.pressed.load()) {
        player_moved = true;
        player->GetComponent<Physics>()->SetVelocity(
            {-30, player->GetComponent<Physics>()->GetVelocity().y}); // Move right
    }

    if (app->key_map->key_up.pressed.load()) {
        player_moved = true;
        player->GetComponent<Physics>()->SetVelocity(
            {player->GetComponent<Physics>()->GetVelocity().x, -30});
    }

    if (!player_moved) {
        player->GetComponent<Physics>()->SetVelocity(
            {0, player->GetComponent<Physics>()->GetVelocity().y});
    }
}

void UpdatePlatforms(Entity *platform) {
    Position platform_position = platform->GetComponent<Transform>()->GetPosition();
    Velocity platform_velocity = platform->GetComponent<Physics>()->GetVelocity();
    float platform_velocity_val = 20;

    if (platform->GetName() == "moving_platform_1") {
        if (platform_position.y < 100) {
            platform->GetComponent<Physics>()->SetVelocity({0, platform_velocity_val});
        } else if (platform_position.y > 900) {
            platform->GetComponent<Physics>()->SetVelocity({0, -platform_velocity_val});
        }

    } else if (platform->GetName() == "moving_platform_2") {
        if (platform_position.x < 2100) {
            platform->GetComponent<Physics>()->SetVelocity({platform_velocity_val, 0});
        } else if (platform_position.x > 2650) {
            platform->GetComponent<Physics>()->SetVelocity({-platform_velocity_val, 0});
        }
    }
}

std::vector<Entity *> CreateGround() {
    std::vector<Entity *> ground_blocks;
    Color ground_color = {50, 200, 100, 255};

    Entity *ground_block1 = new Entity("ground_1", EntityCategory::Stationary);
    ground_block1->AddComponent<Transform>();
    ground_block1->AddComponent<Collision>();
    ground_block1->AddComponent<Render>();
    ground_block1->AddComponent<Network>();
    ground_block1->GetComponent<Transform>()->SetPosition({-500, float(window_size.height) - 200});
    ground_block1->GetComponent<Transform>()->SetSize({1300, 200});
    ground_block1->GetComponent<Render>()->SetColor(ground_color);
    ground_block1->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block1);

    Entity *ground_block2 = new Entity("ground_2", EntityCategory::Stationary);
    ground_block2->AddComponent<Transform>();
    ground_block2->AddComponent<Collision>();
    ground_block2->AddComponent<Render>();
    ground_block2->AddComponent<Network>();
    ground_block2->GetComponent<Transform>()->SetPosition({800, float(window_size.height) - 400});
    ground_block2->GetComponent<Transform>()->SetSize({200, 400});
    ground_block2->GetComponent<Render>()->SetColor(ground_color);
    ground_block2->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block2);

    Entity *ground_block3 = new Entity("ground_3", EntityCategory::Stationary);
    ground_block3->AddComponent<Transform>();
    ground_block3->AddComponent<Collision>();
    ground_block3->AddComponent<Render>();
    ground_block3->AddComponent<Network>();
    ground_block3->GetComponent<Transform>()->SetPosition({1000, float(window_size.height) - 600});
    ground_block3->GetComponent<Transform>()->SetSize({200, 600});
    ground_block3->GetComponent<Render>()->SetColor(ground_color);
    ground_block3->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block3);

    Entity *ground_block4 = new Entity("ground_4", EntityCategory::Stationary);
    ground_block4->AddComponent<Transform>();
    ground_block4->AddComponent<Collision>();
    ground_block4->AddComponent<Render>();
    ground_block4->AddComponent<Network>();
    ground_block4->GetComponent<Transform>()->SetPosition({1200, float(window_size.height) - 800});
    ground_block4->GetComponent<Transform>()->SetSize({200, 800});
    ground_block4->GetComponent<Render>()->SetColor(ground_color);
    ground_block4->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block4);

    Entity *ground_block5 = new Entity("ground_5", EntityCategory::Stationary);
    ground_block5->AddComponent<Transform>();
    ground_block5->AddComponent<Collision>();
    ground_block5->AddComponent<Render>();
    ground_block5->AddComponent<Network>();
    ground_block5->GetComponent<Transform>()->SetPosition({1800, float(window_size.height) - 800});
    ground_block5->GetComponent<Transform>()->SetSize({200, 800});
    ground_block5->GetComponent<Render>()->SetColor(ground_color);
    ground_block5->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block5);

    Entity *ground_block6 = new Entity("ground_6", EntityCategory::Stationary);
    ground_block6->AddComponent<Transform>();
    ground_block6->AddComponent<Collision>();
    ground_block6->AddComponent<Render>();
    ground_block6->AddComponent<Network>();
    ground_block6->GetComponent<Transform>()->SetPosition({2000, float(window_size.height) - 600});
    ground_block6->GetComponent<Transform>()->SetSize({200, 600});
    ground_block6->GetComponent<Render>()->SetColor(ground_color);
    ground_block6->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block6);

    Entity *ground_block7 = new Entity("ground_7", EntityCategory::Stationary);
    ground_block7->AddComponent<Transform>();
    ground_block7->AddComponent<Collision>();
    ground_block7->AddComponent<Render>();
    ground_block7->AddComponent<Network>();
    ground_block7->GetComponent<Transform>()->SetPosition({2200, float(window_size.height) - 400});
    ground_block7->GetComponent<Transform>()->SetSize({200, 400});
    ground_block7->GetComponent<Render>()->SetColor(ground_color);
    ground_block7->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block7);

    Entity *ground_block8 = new Entity("ground_8", EntityCategory::Stationary);
    ground_block8->AddComponent<Transform>();
    ground_block8->AddComponent<Collision>();
    ground_block8->AddComponent<Render>();
    ground_block8->AddComponent<Network>();
    ground_block8->GetComponent<Transform>()->SetPosition({2400, float(window_size.height) - 200});
    ground_block8->GetComponent<Transform>()->SetSize({1300, 200});
    ground_block8->GetComponent<Render>()->SetColor(ground_color);
    ground_block8->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block8);

    return ground_blocks;
}

std::vector<Entity *> CreatePlatforms() {
    std::vector<Entity *> platforms;

    Entity *static_platform_1 = new Entity("static_platform_1", EntityCategory::Stationary);
    static_platform_1->AddComponent<Transform>();
    static_platform_1->AddComponent<Render>();
    static_platform_1->AddComponent<Network>();
    static_platform_1->GetComponent<Transform>()->SetPosition(
        {400, float(window_size.height - 500)});
    static_platform_1->GetComponent<Transform>()->SetSize({200, 150});
    static_platform_1->GetComponent<Render>()->SetColor({0, 0, 0, 255});
    static_platform_1->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    platforms.push_back(static_platform_1);

    Entity *static_platform_2 = new Entity("static_platform_2", EntityCategory::Stationary);
    static_platform_2->AddComponent<Transform>();
    static_platform_2->AddComponent<Render>();
    static_platform_2->AddComponent<Network>();
    static_platform_2->GetComponent<Transform>()->SetPosition(
        {2600, float(window_size.height - 500)});
    static_platform_2->GetComponent<Transform>()->SetSize({300, 100});
    static_platform_2->GetComponent<Render>()->SetColor({130, 108, 108, 255});
    static_platform_2->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    platforms.push_back(static_platform_2);

    Entity *static_platform_3 = new Entity("static_platform_3", EntityCategory::Stationary);
    static_platform_3->AddComponent<Transform>();
    static_platform_3->AddComponent<Render>();
    static_platform_3->AddComponent<Network>();
    static_platform_3->GetComponent<Transform>()->SetPosition(
        {2750, float(window_size.height - 800)});
    static_platform_3->GetComponent<Transform>()->SetSize({700, 200});
    static_platform_3->GetComponent<Render>()->SetColor({61, 12, 71, 255});
    static_platform_3->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    platforms.push_back(static_platform_3);

    Entity *static_platform_4 = new Entity("static_platform_4", EntityCategory::Stationary);
    static_platform_4->AddComponent<Transform>();
    static_platform_4->AddComponent<Render>();
    static_platform_4->AddComponent<Network>();
    static_platform_4->GetComponent<Transform>()->SetPosition(
        {3000, float(window_size.height - 1100)});
    static_platform_4->GetComponent<Transform>()->SetSize({400, 150});
    static_platform_4->GetComponent<Render>()->SetColor({189, 161, 49, 255});
    static_platform_4->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    platforms.push_back(static_platform_4);

    Entity *moving_platform_1 = new Entity("moving_platform_1", EntityCategory::Moving);
    moving_platform_1->AddComponent<Transform>();
    moving_platform_1->AddComponent<Physics>();
    moving_platform_1->AddComponent<Render>();
    moving_platform_1->AddComponent<Network>();
    moving_platform_1->AddComponent<Handler>();
    moving_platform_1->GetComponent<Transform>()->SetPosition(
        {1550, float(window_size.height - 900)});
    moving_platform_1->GetComponent<Physics>()->SetVelocity({0, 20});
    moving_platform_1->GetComponent<Transform>()->SetSize({100, 100});
    moving_platform_1->GetComponent<Render>()->SetColor({79, 78, 77, 255});
    moving_platform_1->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    moving_platform_1->GetComponent<Handler>()->SetCallback(UpdatePlatforms);
    platforms.push_back(moving_platform_1);

    Entity *moving_platform_2 = new Entity("moving_platform_2", EntityCategory::Moving);
    moving_platform_2->AddComponent<Transform>();
    moving_platform_2->AddComponent<Physics>();
    moving_platform_2->AddComponent<Render>();
    moving_platform_2->AddComponent<Network>();
    moving_platform_2->AddComponent<Handler>();
    moving_platform_2->GetComponent<Transform>()->SetPosition(
        {2100, float(window_size.height - 1000)});
    moving_platform_2->GetComponent<Physics>()->SetVelocity({20, 0});
    moving_platform_2->GetComponent<Transform>()->SetSize({200, 100});
    moving_platform_2->GetComponent<Render>()->SetColor({79, 78, 77, 255});
    moving_platform_2->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    moving_platform_2->GetComponent<Handler>()->SetCallback(UpdatePlatforms);
    platforms.push_back(moving_platform_2);

    return platforms;
}

Entity *CreatePlayer() {
    Entity *player = new Entity("player", EntityCategory::Controllable);
    player->AddComponent<Transform>();
    player->AddComponent<Physics>();
    player->AddComponent<Render>();
    player->AddComponent<Collision>();
    player->AddComponent<Network>();
    player->AddComponent<Handler>();

    // player->GetComponent<Transform>()->SetPosition({300, 300});
    player->GetComponent<Transform>()->SetSize({50, 100});

    player->GetComponent<Physics>()->SetAcceleration({0, 10});
    player->GetComponent<Render>()->SetTextureTemplate("assets/player_{}.png");
    player->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    player->GetComponent<Handler>()->SetCallback(UpdatePlayer);

    return player;
}

void CreateSideBoundaries(Engine *game_engine) {
    // left side boundary
    game_engine->AddSideBoundary(Position{100, -1000}, Size{20, 3000});
    // right side boundary
    game_engine->AddSideBoundary(Position{float(window_size.width - 400), -1000}, Size{20, 3000});
    // top boundary
    game_engine->AddSideBoundary(Position{-600, float(window_size.height - 1000)}, Size{4000, 20});
    // bottom boundary
    game_engine->AddSideBoundary(Position{-600, float(window_size.height - 100)}, Size{4000, 20});
}

void CreateDeathZones(Engine *game_engine) {
    game_engine->AddDeathZone(Position{-500, -1000}, Size{20, 3000});
    game_engine->AddDeathZone(Position{3700, -1000}, Size{20, 3000});
    game_engine->AddDeathZone(Position{-500, float(window_size.height)}, Size{4000, 20});
}

void CreateSpawnPoints(Engine *game_engine) {
    game_engine->AddSpawnPoint(Position{300, 300}, Size{10, 10});
    game_engine->AddSpawnPoint(Position{400, 250}, Size{10, 10});
    game_engine->AddSpawnPoint(Position{500, 200}, Size{10, 10});
    game_engine->AddSpawnPoint(Position{600, 150}, Size{10, 10});
}

// std::vector<Entity *> CreateFire() {}

std::vector<Entity *> CreateEntities(Engine *game_engine) {
    std::vector<Entity *> entities;

    std::vector<Entity *> ground = CreateGround();
    std::vector<Entity *> platforms = CreatePlatforms();
    CreateSideBoundaries(game_engine);
    CreateSpawnPoints(game_engine);
    CreateDeathZones(game_engine);
    Entity *player = CreatePlayer();

    entities.push_back(player);
    entities.insert(entities.end(), ground.begin(), ground.end());
    entities.insert(entities.end(), platforms.begin(), platforms.end());

    for (Entity *entity : entities) {
        if (network_info.mode == NetworkMode::PeerToPeer) {
            if (entity->GetComponent<Network>()) {
                if (entity->GetComponent<Network>()->GetOwner() == NetworkRole::Server) {
                    entity->GetComponent<Network>()->SetOwner(NetworkRole::Host);
                }
                if (entity->GetComponent<Network>()->GetOwner() == NetworkRole::Client) {
                    entity->GetComponent<Network>()->SetOwner(NetworkRole::Peer);
                }
            }
        }
    }
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
    int max_player_count = 100, texture_count = 4;

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
    if (network_info.id > max_player_count) {
        Log(LogLevel::Error, "More than %d players spotted: EXITING THE GAME. Player ID: %d",
            max_player_count, network_info.id);
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