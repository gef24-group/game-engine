// This game will not be compiled or analyzed due to breaking API changes
// Please revert to the following commit to compile this game
// dbb2658bd097921c7fd53c994fdd3806b5d81c47

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
#include <vector>

NetworkInfo network_info;
Size window_size;

void Update(std::vector<Entity *> *entities) {}

void UpdateEntity(Entity *entity) {
    Position entity_position = entity->GetComponent<Transform>()->GetPosition();

    if (entity_position.x < 50) {
        entity->GetComponent<Physics>()->SetVelocity({10, 0});
    } else if (entity_position.x > 1000) {
        entity->GetComponent<Physics>()->SetVelocity({-10, 0});
    }
}

void UpdatePlayer(Entity *player) {}

std::vector<Entity *> CreateEntities(Engine *game_engine, int num_entities) {
    std::vector<Entity *> entities;

    Entity *player = new Entity("player", EntityCategory::Controllable);
    player->AddComponent<Transform>();
    player->AddComponent<Physics>();
    player->AddComponent<Render>();
    player->AddComponent<Collision>();
    player->AddComponent<Network>();
    player->AddComponent<Handler>();
    player->GetComponent<Render>()->SetTextureTemplate("assets/player_{}.png");
    player->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    player->GetComponent<Handler>()->SetCallback(UpdatePlayer);

    entities.push_back(player);

    for (int i = 0; i < num_entities; i++) {
        Entity *entity =
            new Entity("moving_entity_" + std::to_string(i + 1), EntityCategory::Moving);
        entity->AddComponent<Transform>();
        entity->AddComponent<Physics>();
        entity->AddComponent<Render>();
        entity->AddComponent<Network>();
        entity->AddComponent<Handler>();

        entity->GetComponent<Transform>()->SetPosition({50, float(20 * i)});
        entity->GetComponent<Transform>()->SetSize({10, 10});
        entity->GetComponent<Physics>()->SetVelocity({10, 0});
        entity->GetComponent<Network>()->SetOwner(NetworkRole::Server);
        entity->GetComponent<Handler>()->SetCallback(UpdateEntity);

        entities.push_back(entity);
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

int GetMovingCountFromCLI(int argc, char *args[]) {
    int count = -1;

    for (int i = 1; i < argc; i++) {
        std::string arg = args[i];

        if (arg == "--count" && i + 1 < argc) {
            count = std::atoi(args[i + 1]);
        }
    }

    return count;
}

int main(int argc, char *args[]) {
    std::string game_title = "Comparing performance: Varying moving objects";
    int max_player_count = 100;

    // Initializing the Game Engine
    Engine game_engine;
    if (!SetEngineCLIOptions(&game_engine, argc, args)) {
        return 1;
    }

    int moving_count = GetMovingCountFromCLI(argc, args);

    if (!game_engine.Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    network_info = game_engine.GetNetworkInfo();
    if (network_info.id > max_player_count) {
        exit(0);
    }

    Color background_color = Color{165, 200, 255, 255};
    game_engine.SetBackgroundColor(background_color);
    game_engine.SetTitle(game_title);

    window_size = GetWindowSize();

    std::vector<Entity *> entities = CreateEntities(&game_engine, moving_count);
    AddObjectsToEngine(entities, &game_engine);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");

    // Add Game Cleanup code (deallocating pointers)
    DestroyEntities(entities);
    return 0;
}