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

void Update(std::vector<Entity *> *entities) {

    // std::cout << "Update called" << std::endl; // Log for debugging

    // Movement variables for ground_block6

    static Entity *ground_block6 = nullptr;
    static Entity *ground_block8 = nullptr;
    static Entity *player = nullptr;

    // Find the entity with the name "ground_6"
    if (player == nullptr && ground_block6 == nullptr && ground_block8 == nullptr) {
        for (Entity *entity : *entities) {
            if (entity->GetName() == "ground_6") { // Ensure the name is correct
                ground_block6 = entity;
            }

            if (entity->GetName() == "ground_8") { // Ensure the name is correct
                ground_block8 = entity;
                // std::cout << "Found ground_block8!" << std::endl;
            }

            if (entity->GetName() == "player") {
                player = entity;
            }
        }
    }

    if (ground_block6 != nullptr) {
        // Get the initial Y position of ground_block6
        // static float initial_y = ground_block6->GetComponent<Transform>()->GetPosition().y;
        float max_height = 500;  // Max height it can move up
        float min_height = 1031; // Initial position is the minimum height

        // Get the current position of ground_block6
        auto current_pos = ground_block6->GetComponent<Transform>()->GetPosition();

        // Log the current position for debugging
        // std::cout << "Current Position: (" << current_pos.x << ", " << current_pos.y << ")"
        //          << std::endl;

        // Check the position and update the velocity
        if (current_pos.y > min_height) {
            // Reverse movement direction by setting the negative velocity
            ground_block6->GetComponent<Physics>()->SetVelocity(
                Velocity{0, -ground_block6->GetComponent<Physics>()->GetVelocity().y});
            // std::cout << "Reversing direction" << std::endl; // Log direction change
        } else if (current_pos.y <= max_height) {
            // Set upward velocity when not at limits
            ground_block6->GetComponent<Physics>()->SetVelocity(
                Velocity{0, 20.0f}); // Change 2.0f as needed
        }
    }

    if (ground_block8 != nullptr) {
        // Get the initial Y position of ground_block6
        // static float initial_y = ground_block6->GetComponent<Transform>()->GetPosition().y;
        float left_width = 2430;  // Max height it can move up
        float right_width = 2850; // Initial position is the minimum height

        // Get the current position of ground_block6
        auto current_pos = ground_block8->GetComponent<Transform>()->GetPosition();

        // Log the current position for debugging
        // std::cout << "Current Position: (" << current_pos.x << ", " << current_pos.y << ")"
        //          << std::endl;

        // Check the position and update the velocity
        if (current_pos.x >= right_width) {
            // Reverse movement direction by setting the negative velocity
            ground_block8->GetComponent<Physics>()->SetVelocity(
                Velocity{-ground_block8->GetComponent<Physics>()->GetVelocity().x, 0});
            // std::cout << "Left direction" << std::endl; // Log direction change
        } else if (current_pos.x < left_width) {
            // Set upward velocity when not at limits
            ground_block8->GetComponent<Physics>()->SetVelocity(
                Velocity{10.0f, 0}); // Change 2.0f as needed
        }
    }

    if (player != nullptr) {
        for (Entity *entity : player->GetComponent<Collision>()->GetColliders()) {

            // std::cout << entity->GetName();
            if (entity->GetName() == "miscelleanous_entity4") {
                std::cout << "Collided";
            }
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
    Color ground_color = {50, 200, 100, 255};

    Entity *ground_block1 = new Entity("ground_1", EntityCategory::Stationary);
    ground_block1->AddComponent<Transform>();
    ground_block1->AddComponent<Collision>();
    ground_block1->AddComponent<Render>();
    ground_block1->AddComponent<Network>();
    ground_block1->GetComponent<Transform>()->SetPosition({-500, float(window_size.height) - 200});
    ground_block1->GetComponent<Transform>()->SetSize({600, 300});
    ground_block1->GetComponent<Render>()->SetTexture("assets/ground.png");
    // ground_block1->GetComponent<Render>()->SetColor(ground_color);
    ground_block1->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block1);

    Entity *ground_block2 = new Entity("ground_2", EntityCategory::Stationary);
    ground_block2->AddComponent<Transform>();
    ground_block2->AddComponent<Collision>();
    ground_block2->AddComponent<Render>();
    ground_block2->AddComponent<Network>();
    ground_block2->GetComponent<Transform>()->SetPosition({100, float(window_size.height) - 300});
    ground_block2->GetComponent<Transform>()->SetSize({200, 1200});
    // ground_block2->GetComponent<Render>()->SetTexture("assets/ground2.png");
    ground_block2->GetComponent<Render>()->SetColor(Color{121, 20, 90, 255});
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
    ground_block4->GetComponent<Transform>()->SetPosition({800, float(window_size.height) - 300});
    ground_block4->GetComponent<Transform>()->SetSize({200, 50});
    ground_block4->GetComponent<Render>()->SetColor(ground_color);
    ground_block4->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block4);

    Entity *ground_block5 = new Entity("ground_5", EntityCategory::Stationary);
    ground_block5->AddComponent<Transform>();
    ground_block5->AddComponent<Collision>();
    ground_block5->AddComponent<Render>();
    ground_block5->AddComponent<Network>();
    ground_block5->GetComponent<Transform>()->SetPosition({1100, float(window_size.height) - 200});
    ground_block5->GetComponent<Transform>()->SetSize({300, 300});
    ground_block5->GetComponent<Render>()->SetColor(ground_color);
    ground_block5->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block5);

    Entity *ground_block6 = new Entity("ground_6", EntityCategory::Moving);
    ground_block6->AddComponent<Transform>();
    ground_block6->AddComponent<Collision>();
    ground_block6->AddComponent<Render>();
    ground_block6->AddComponent<Network>();
    ground_block6->AddComponent<Physics>();
    ground_block6->GetComponent<Physics>()->SetVelocity(Velocity{0, -20.0f});
    ground_block6->GetComponent<Transform>()->SetPosition({1450, float(window_size.height) - 50});
    ground_block6->GetComponent<Transform>()->SetSize({300, 50});
    ground_block6->GetComponent<Render>()->SetColor(ground_color);
    ground_block6->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block6);

    Entity *ground_block7 = new Entity("ground_7", EntityCategory::Stationary);
    ground_block7->AddComponent<Transform>();
    ground_block7->AddComponent<Collision>();
    ground_block7->AddComponent<Render>();
    ground_block7->AddComponent<Network>();
    ground_block7->GetComponent<Transform>()->SetPosition({1800, float(window_size.height) - 400});
    ground_block7->GetComponent<Transform>()->SetSize({600, 400});
    ground_block7->GetComponent<Render>()->SetColor(ground_color);
    ground_block7->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block7);

    Entity *ground_block8 = new Entity("ground_8", EntityCategory::Moving);
    ground_block8->AddComponent<Transform>();
    ground_block8->AddComponent<Collision>();
    ground_block8->AddComponent<Render>();
    ground_block8->AddComponent<Network>();
    ground_block8->AddComponent<Physics>();
    ground_block8->GetComponent<Physics>()->SetVelocity(Velocity{10.0f, 0});
    ground_block8->GetComponent<Transform>()->SetPosition({2500, float(window_size.height) - 100});
    ground_block8->GetComponent<Transform>()->SetSize({200, 50});
    ground_block8->GetComponent<Render>()->SetColor(ground_color);
    ground_block8->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block8);

    Entity *ground_block9 = new Entity("ground_9", EntityCategory::Stationary);
    ground_block9->AddComponent<Transform>();
    ground_block9->AddComponent<Collision>();
    ground_block9->AddComponent<Render>();
    ground_block9->AddComponent<Network>();
    ground_block9->AddComponent<Physics>();
    // ground_block9->GetComponent<Physics>()->SetVelocity(Velocity{10.0f, 0});
    ground_block9->GetComponent<Transform>()->SetPosition({3100, float(window_size.height) - 200});
    ground_block9->GetComponent<Transform>()->SetSize({600, 300});
    ground_block9->GetComponent<Render>()->SetColor(ground_color);
    ground_block9->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    ground_blocks.push_back(ground_block9);

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
        {1000, float(window_size.height) - 1105});
    miscelleanous_entity1->GetComponent<Transform>()->SetSize({150, 150});
    miscelleanous_entity1->GetComponent<Render>()->SetTexture("assets/bird.png");

    // miscelleanous_entity1->GetComponent<Render>()->SetColor(ground_color);
    miscelleanous_entity1->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    miscelleanous_entities.push_back(miscelleanous_entity1);

    Entity *miscelleanous_entity2 = new Entity("miscelleanous_entity2", EntityCategory::Stationary);
    miscelleanous_entity2->AddComponent<Transform>();
    miscelleanous_entity2->AddComponent<Collision>();
    miscelleanous_entity2->AddComponent<Render>();
    miscelleanous_entity2->AddComponent<Network>();
    miscelleanous_entity2->GetComponent<Transform>()->SetPosition(
        {2200, float(window_size.height) - 1105});
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
        {1800, float(window_size.height) - 1050});
    miscelleanous_entity3->GetComponent<Transform>()->SetSize({150, 150});
    miscelleanous_entity3->GetComponent<Render>()->SetTexture("assets/sun.png");

    // miscelleanous_entity1->GetComponent<Render>()->SetColor(ground_color);
    miscelleanous_entity3->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    miscelleanous_entities.push_back(miscelleanous_entity3);

    Entity *miscelleanous_entity4 = new Entity("miscelleanous_entity4", EntityCategory::Stationary);
    miscelleanous_entity4->AddComponent<Transform>();
    miscelleanous_entity4->AddComponent<Collision>();
    miscelleanous_entity4->AddComponent<Render>();
    miscelleanous_entity4->AddComponent<Network>();
    miscelleanous_entity4->GetComponent<Transform>()->SetPosition(
        {3550, float(window_size.height) - 326});
    miscelleanous_entity4->GetComponent<Transform>()->SetSize({100, 150});
    miscelleanous_entity4->GetComponent<Render>()->SetTexture("assets/flag.png");

    // miscelleanous_entity1->GetComponent<Render>()->SetColor(ground_color);
    miscelleanous_entity4->GetComponent<Network>()->SetOwner(NetworkRole::Server);
    miscelleanous_entities.push_back(miscelleanous_entity4);

    return miscelleanous_entities;
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
    game_engine->AddDeathZone(Position{-500, -1000}, Size{20, 3000});
    game_engine->AddDeathZone(Position{3700, -1000}, Size{20, 3000});
    game_engine->AddDeathZone(Position{-500, float(window_size.height)}, Size{4000, 20});
}

void CreateSpawnPoints(Engine *game_engine) {
    game_engine->AddSpawnPoint(Position{75, 300}, Size{10, 10});
    game_engine->AddSpawnPoint(Position{400, 100}, Size{10, 10});
}

// std::vector<Entity *> CreateFire() {}

std::vector<Entity *> CreateEntities(Engine *game_engine) {
    std::vector<Entity *> entities;

    std::vector<Entity *> ground = CreateGround();
    std::vector<Entity *> miscelleanous = CreateMiscelleanousEntities();
    CreateSideBoundaries(game_engine);
    CreateSpawnPoints(game_engine);
    CreateDeathZones(game_engine);
    Entity *player = CreatePlayer();

    entities.push_back(player);
    entities.insert(entities.end(), ground.begin(), ground.end());
    entities.insert(entities.end(), miscelleanous.begin(), miscelleanous.end());

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