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
#include <iostream>
#include <string>
#include <vector>

NetworkInfo network_info;
Size window_size;

void Update(std::vector<Entity *> *entities) {}

void UpdatePlatforms(Entity *platform) {
    Position platform_position = platform->GetComponent<Transform>()->GetPosition();
    Velocity platform_velocity = platform->GetComponent<Physics>()->GetVelocity();

    if (platform->GetName() == "platform4") {
        if (platform_position.y < 500) {
            platform->GetComponent<Physics>()->SetVelocity({0, 10.0f});
        }
        if (platform_position.y > 1031) {
            platform->GetComponent<Physics>()->SetVelocity({0, -20.0f});
        }
    } else if (platform->GetName() == "platform5") {
        if (platform_position.x < 2430) {
            platform->GetComponent<Physics>()->SetVelocity({30.0f, 0});
        }
        if (platform_position.x > 2850) {
            platform->GetComponent<Physics>()->SetVelocity({-30.0f, 0});
        }
    }
}

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
    Color ground_color = {0, 0, 0, 255};

    Entity *ground_block1 = new Entity("ground_1", EntityCategory::Stationary);
    ground_block1->AddComponent<Transform>();
    ground_block1->AddComponent<Collision>();
    ground_block1->AddComponent<Render>();
    ground_block1->AddComponent<Network>();
    ground_block1->GetComponent<Transform>()->SetPosition({-500, float(window_size.height) - 200});
    ground_block1->GetComponent<Transform>()->SetSize({600, 300});
    ground_block1->GetComponent<Render>()->SetColor(ground_color);
    ground_block1->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block1);

    Entity *ground_block2 = new Entity("ground_2", EntityCategory::Stationary);
    ground_block2->AddComponent<Transform>();
    ground_block2->AddComponent<Collision>();
    ground_block2->AddComponent<Render>();
    ground_block2->AddComponent<Network>();
    ground_block2->GetComponent<Transform>()->SetPosition({100, float(window_size.height) - 300});
    ground_block2->GetComponent<Transform>()->SetSize({200, 1200});
    ground_block2->GetComponent<Render>()->SetColor(ground_color);
    ground_block2->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block2);

    Entity *ground_block3 = new Entity("ground_3", EntityCategory::Stationary);
    ground_block3->AddComponent<Transform>();
    ground_block3->AddComponent<Collision>();
    ground_block3->AddComponent<Render>();
    ground_block3->AddComponent<Network>();
    ground_block3->GetComponent<Transform>()->SetPosition({300, float(window_size.height) - 200});
    ground_block3->GetComponent<Transform>()->SetSize({400, 300});
    ground_block3->GetComponent<Render>()->SetColor(ground_color);
    ground_block3->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block3);

    Entity *ground_block4 = new Entity("ground_4", EntityCategory::Stationary);
    ground_block4->AddComponent<Transform>();
    ground_block4->AddComponent<Collision>();
    ground_block4->AddComponent<Render>();
    ground_block4->AddComponent<Network>();
    ground_block4->GetComponent<Transform>()->SetPosition({1100, float(window_size.height) - 200});
    ground_block4->GetComponent<Transform>()->SetSize({300, 300});
    ground_block4->GetComponent<Render>()->SetColor(ground_color);
    ground_block4->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block4);

    Entity *ground_block5 = new Entity("ground_5", EntityCategory::Stationary);
    ground_block5->AddComponent<Transform>();
    ground_block5->AddComponent<Collision>();
    ground_block5->AddComponent<Render>();
    ground_block5->AddComponent<Network>();
    ground_block5->GetComponent<Transform>()->SetPosition({1800, float(window_size.height) - 400});
    ground_block5->GetComponent<Transform>()->SetSize({600, 400});
    ground_block5->GetComponent<Render>()->SetColor(ground_color);
    ground_block5->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block5);

    Entity *ground_block6 = new Entity("ground_6", EntityCategory::Stationary);
    ground_block6->AddComponent<Transform>();
    ground_block6->AddComponent<Collision>();
    ground_block6->AddComponent<Render>();
    ground_block6->AddComponent<Network>();
    ground_block6->AddComponent<Physics>();
    ground_block6->GetComponent<Transform>()->SetPosition({3100, float(window_size.height) - 200});
    ground_block6->GetComponent<Transform>()->SetSize({600, 300});
    ground_block6->GetComponent<Render>()->SetColor(ground_color);
    ground_block6->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block6);

    return ground_blocks;
}

std::vector<Entity *> CreateMiscelleanousEntities() {
    std::vector<Entity *> miscelleanous_entities;

    Entity *miscelleanous_entity1 = new Entity("miscelleanous_entity1", EntityCategory::Stationary);
    miscelleanous_entity1->AddComponent<Transform>();
    miscelleanous_entity1->AddComponent<Collision>();
    miscelleanous_entity1->AddComponent<Render>();
    miscelleanous_entity1->AddComponent<Network>();
    miscelleanous_entity1->GetComponent<Transform>()->SetPosition(
        {1000, float(window_size.height) - 1080});
    miscelleanous_entity1->GetComponent<Transform>()->SetSize({150, 150});
    miscelleanous_entity1->GetComponent<Render>()->SetTexture("assets/bird.png");
    miscelleanous_entity1->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    miscelleanous_entities.push_back(miscelleanous_entity1);

    Entity *miscelleanous_entity2 = new Entity("miscelleanous_entity2", EntityCategory::Stationary);
    miscelleanous_entity2->AddComponent<Transform>();
    miscelleanous_entity2->AddComponent<Collision>();
    miscelleanous_entity2->AddComponent<Render>();
    miscelleanous_entity2->AddComponent<Network>();
    miscelleanous_entity2->GetComponent<Transform>()->SetPosition(
        {2200, float(window_size.height) - 1080});
    miscelleanous_entity2->GetComponent<Transform>()->SetSize({150, 150});
    miscelleanous_entity2->GetComponent<Render>()->SetTexture("assets/bird.png");

    // miscelleanous_entity1->GetComponent<Render>()->SetColor(ground_color);
    miscelleanous_entity2->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    miscelleanous_entities.push_back(miscelleanous_entity2);

    Entity *miscelleanous_entity3 = new Entity("miscelleanous_entity3", EntityCategory::Stationary);
    miscelleanous_entity3->AddComponent<Transform>();
    miscelleanous_entity3->AddComponent<Collision>();
    miscelleanous_entity3->AddComponent<Render>();
    miscelleanous_entity3->AddComponent<Network>();
    miscelleanous_entity3->GetComponent<Transform>()->SetPosition(
        {1800, float(window_size.height) - 1020});
    miscelleanous_entity3->GetComponent<Transform>()->SetSize({150, 150});
    miscelleanous_entity3->GetComponent<Render>()->SetTexture("assets/sun.png");

    // miscelleanous_entity1->GetComponent<Render>()->SetColor(ground_color);
    miscelleanous_entity3->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    miscelleanous_entities.push_back(miscelleanous_entity3);

    return miscelleanous_entities;
}

std::vector<Entity *> CreatePlatforms() {
    std::vector<Entity *> platforms;
    // ADD CODE HERE

    Entity *platform1 = new Entity("platform1", EntityCategory::Stationary);
    platform1->AddComponent<Transform>();
    platform1->AddComponent<Render>();
    platform1->AddComponent<Network>();
    platform1->GetComponent<Transform>()->SetPosition({800, float(window_size.height) - 300});
    platform1->GetComponent<Transform>()->SetSize({200, 50});
    platform1->GetComponent<Render>()->SetColor(Color{2, 55, 12, 255});
    platform1->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    platforms.push_back(platform1);

    Entity *platform2 = new Entity("platform2", EntityCategory::Stationary);
    platform2->AddComponent<Transform>();
    platform2->AddComponent<Render>();
    platform2->AddComponent<Network>();
    platform2->GetComponent<Transform>()->SetPosition({1100, float(window_size.height) - 700});
    platform2->GetComponent<Transform>()->SetSize({175, 150});
    platform2->GetComponent<Render>()->SetColor(Color{198, 169, 122, 255});
    platform2->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    platforms.push_back(platform2);

    Entity *platform3 = new Entity("platform3", EntityCategory::Stationary);
    platform3->AddComponent<Transform>();
    platform3->AddComponent<Render>();
    platform3->AddComponent<Network>();
    platform3->GetComponent<Transform>()->SetPosition({2600, float(window_size.height) - 900});
    platform3->GetComponent<Transform>()->SetSize({300, 100});
    platform3->GetComponent<Render>()->SetColor(Color{161, 164, 70, 255});
    platform3->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    platforms.push_back(platform3);

    Entity *platform4 = new Entity("platform4", EntityCategory::Moving);
    platform4->AddComponent<Transform>();
    platform4->AddComponent<Render>();
    platform4->AddComponent<Network>();
    platform4->AddComponent<Physics>();
    platform4->AddComponent<Handler>();
    platform4->GetComponent<Physics>()->SetVelocity(Velocity{0, -20.0f});
    platform4->GetComponent<Transform>()->SetPosition({1450, float(window_size.height) - 50});
    platform4->GetComponent<Transform>()->SetSize({300, 50});
    platform4->GetComponent<Render>()->SetColor(Color{153, 0, 76, 255});
    platform4->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    platform4->GetComponent<Handler>()->SetCallback(UpdatePlatforms);
    platforms.push_back(platform4);

    Entity *platform5 = new Entity("platform5", EntityCategory::Moving);
    platform5->AddComponent<Transform>();
    platform5->AddComponent<Render>();
    platform5->AddComponent<Network>();
    platform5->AddComponent<Physics>();
    platform5->AddComponent<Handler>();
    platform5->GetComponent<Physics>()->SetVelocity(Velocity{10.0f, 0});
    platform5->GetComponent<Transform>()->SetPosition({2500, float(window_size.height) - 100});
    platform5->GetComponent<Transform>()->SetSize({200, 50});
    platform5->GetComponent<Render>()->SetColor(Color{30, 35, 156, 255});
    platform5->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    platform5->GetComponent<Handler>()->SetCallback(UpdatePlatforms);
    platforms.push_back(platform5);

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
    player->GetComponent<Transform>()->SetSize({100, 100});

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
    game_engine->AddDeathZone(Position{-505, -1005}, Size{20, 3000});
    game_engine->AddDeathZone(Position{3705, -1005}, Size{20, 3000});
    game_engine->AddDeathZone(Position{-505, float(window_size.height)}, Size{4000, 20});
}

void CreateSpawnPoints(Engine *game_engine) {
    game_engine->AddSpawnPoint(Position{75, 300}, Size{10, 10});
    game_engine->AddSpawnPoint(Position{400, 100}, Size{10, 10});
    game_engine->AddSpawnPoint(Position{520, 300}, Size{10, 10});
}

std::vector<Entity *> CreateEntities(Engine *game_engine) {
    std::vector<Entity *> entities;

    std::vector<Entity *> ground = CreateGround();
    std::vector<Entity *> miscelleanous = CreateMiscelleanousEntities();
    std::vector<Entity *> platforms = CreatePlatforms();

    CreateSideBoundaries(game_engine);
    CreateSpawnPoints(game_engine);
    CreateDeathZones(game_engine);
    Entity *player = CreatePlayer();

    entities.push_back(player);
    entities.insert(entities.end(), ground.begin(), ground.end());
    entities.insert(entities.end(), miscelleanous.begin(), miscelleanous.end());
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
    std::string game_title = "Mitesh's CSC581 HW3 Game: Platformer";
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

    Color background_color = Color{135, 206, 235, 255};
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