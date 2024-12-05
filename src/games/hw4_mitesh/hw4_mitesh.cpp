// This game will not be compiled or analyzed due to breaking API changes
// Please revert to the following commit to compile this game
// 9af70f40c9a0b342d480f27e040742551d40f4da

#include "Collision.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Event.hpp"
#include "Handler.hpp"
#include "Network.hpp"
#include "Physics.hpp"
#include "Render.hpp"
#include "SDL_scancode.h"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <atomic>
#include <string>
#include <vector>

NetworkInfo network_info;
Size window_size;

struct PlayerEvent {
    bool move_left;
    bool move_right;
    bool jump;
    bool dash_left;
    bool dash_right;
    bool power_jump;
} player_event;

void Update(std::vector<Entity *> &entities) {}

void AssignOperationsToKeys() {
    Engine::GetInstance().BindPauseKey(SDL_SCANCODE_P);
    Engine::GetInstance().BindSpeedDownKey(SDL_SCANCODE_COMMA);
    Engine::GetInstance().BindSpeedUpKey(SDL_SCANCODE_PERIOD);
    Engine::GetInstance().BindDisplayScalingKey(SDL_SCANCODE_X);
    Engine::GetInstance().BindHiddenZoneKey(SDL_SCANCODE_Z);

    Engine::GetInstance().RegisterInputChord(1, {SDL_SCANCODE_LEFT, SDL_SCANCODE_SPACE});
    Engine::GetInstance().RegisterInputChord(2, {SDL_SCANCODE_RIGHT, SDL_SCANCODE_SPACE});
    Engine::GetInstance().RegisterInputChord(3, {SDL_SCANCODE_UP, SDL_SCANCODE_SPACE});
}

// Pass a nullptr if the input event was not a chord event so that the chord actions are disabled.
void HandlePlayerChordInput(InputEvent *event) {
    if (event) {
        bool pressed = event->pressed;

        switch (event->chord_id) {
        case 1:
            player_event.dash_left = pressed;
            break;
        case 2:
            player_event.dash_right = pressed;
            break;
        case 3:
            player_event.power_jump = pressed;
            break;
        default:
            break;
        }
    } else {
        // Chord release events are not raised
        player_event.dash_left = player_event.dash_right = player_event.power_jump = false;
    }
}

void HandlePlayerSingleInput(InputEvent *event) {
    HandlePlayerChordInput(nullptr);
    bool pressed = event->pressed;
    SDL_Scancode key = event->key;

    switch (key) {
    case SDL_SCANCODE_LEFT:
        player_event.move_left = pressed;
        break;
    case SDL_SCANCODE_RIGHT:
        player_event.move_right = pressed;
        break;
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_SPACE:
        player_event.jump = pressed;
        break;

    default:
        break;
    }
}

void HandlePlayerEvent(Entity &player, Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));

    if (input_event) {
        switch (input_event->type) {
        case InputEventType::Single:
            HandlePlayerSingleInput(input_event);
            break;
        case InputEventType::Chord:
            HandlePlayerChordInput(input_event);
            break;
        default:
            break;
        }
    }
}

void UpdatePlatforms(Entity &platform) {
    Position platform_position = platform.GetComponent<Transform>()->GetPosition();
    Velocity platform_velocity = platform.GetComponent<Physics>()->GetVelocity();

    if (platform.GetName() == "platform4") {
        if (platform_position.y < 500) {
            platform.GetComponent<Physics>()->SetVelocity({0, 10.0f});
        }
        if (platform_position.y > 1031) {
            platform.GetComponent<Physics>()->SetVelocity({0, -20.0f});
        }
    } else if (platform.GetName() == "platform5") {
        if (platform_position.x < 2430) {
            platform.GetComponent<Physics>()->SetVelocity({30.0f, 0});
        }
        if (platform_position.x > 2850) {
            platform.GetComponent<Physics>()->SetVelocity({-30.0f, 0});
        }
    }
}

void UpdatePlayer(Entity &player) {
    bool player_moved = false;

    if (player_event.move_right) {
        player_moved = true;
        player.GetComponent<Physics>()->SetVelocity(
            {20, player.GetComponent<Physics>()->GetVelocity().y}); // Move left
    } else if (player_event.move_left) {
        player_moved = true;
        player.GetComponent<Physics>()->SetVelocity(
            {-20, player.GetComponent<Physics>()->GetVelocity().y}); // Move right
    }

    if (player_event.jump) {
        // Log(LogLevel::Info, "Regular jump");
        player_moved = true;
        player.GetComponent<Physics>()->SetVelocity(
            {player.GetComponent<Physics>()->GetVelocity().x, -30});
    }
    if (player_event.dash_left) {
        // Log(LogLevel::Info, "Left dash!");
        player.GetComponent<Physics>()->SetVelocity(
            {-90, player.GetComponent<Physics>()->GetVelocity().y});
        player_moved = true;
    }
    if (player_event.dash_right) {
        // Log(LogLevel::Info, "Right dash!");
        player.GetComponent<Physics>()->SetVelocity(
            {90, player.GetComponent<Physics>()->GetVelocity().y});
        player_moved = true;
    }
    if (player_event.power_jump) {
        // Log(LogLevel::Info, "Power jump!");
        player.GetComponent<Physics>()->SetVelocity(
            {player.GetComponent<Physics>()->GetVelocity().x, -100});
        player_moved = true;
    }

    if (!player_moved) {
        player.GetComponent<Physics>()->SetVelocity(
            {0, player.GetComponent<Physics>()->GetVelocity().y});
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
    miscelleanous_entity1->GetComponent<Render>()->SetTexture("bird.png");
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
    miscelleanous_entity2->GetComponent<Render>()->SetTexture("bird.png");

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
    miscelleanous_entity3->GetComponent<Render>()->SetTexture("sun.png");

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
    platform4->GetComponent<Handler>()->SetUpdateCallback(UpdatePlatforms);
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
    platform5->GetComponent<Handler>()->SetUpdateCallback(UpdatePlatforms);
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
    player->GetComponent<Render>()->SetTextureTemplate("player_{}.png");
    player->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    player->GetComponent<Handler>()->SetUpdateCallback(UpdatePlayer);
    player->GetComponent<Handler>()->SetEventCallback(HandlePlayerEvent);

    return player;
}

void CreateSideBoundaries() {
    // left side boundary
    Engine::GetInstance().AddSideBoundary(Position{100, -1000}, Size{20, 3000});
    // right side boundary
    Engine::GetInstance().AddSideBoundary(Position{float(window_size.width - 400), -1000},
                                          Size{20, 3000});
    // top boundary
    Engine::GetInstance().AddSideBoundary(Position{-600, float(window_size.height - 1000)},
                                          Size{4000, 20});
    // bottom boundary
    Engine::GetInstance().AddSideBoundary(Position{-600, float(window_size.height - 100)},
                                          Size{4000, 20});
}

void CreateDeathZones() {
    Engine::GetInstance().AddDeathZone(Position{-505, -1005}, Size{20, 3000});
    Engine::GetInstance().AddDeathZone(Position{3705, -1005}, Size{20, 3000});
    Engine::GetInstance().AddDeathZone(Position{-505, float(window_size.height)}, Size{4000, 20});
}

void CreateSpawnPoints() {
    Engine::GetInstance().AddSpawnPoint(Position{75, 300}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{400, 100}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{520, 300}, Size{10, 10});
}

std::vector<Entity *> CreateEntities() {
    std::vector<Entity *> entities;

    std::vector<Entity *> ground = CreateGround();
    std::vector<Entity *> miscelleanous = CreateMiscelleanousEntities();
    std::vector<Entity *> platforms = CreatePlatforms();

    CreateSideBoundaries();
    CreateSpawnPoints();
    CreateDeathZones();
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

void AddObjectsToEngine(std::vector<Entity *> entities) {
    for (Entity *entity : entities) {
        Engine::GetInstance().AddEntity(entity);
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

    if (!SetEngineCLIOptions(argc, args)) {
        return 1;
    }

    if (!Engine::GetInstance().Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    AssignOperationsToKeys();
    Engine::GetInstance().SetPlayerTextures(texture_count);
    Engine::GetInstance().SetMaxPlayers(max_player_count);
    Engine::GetInstance().SetShowPlayerBorder(true);

    network_info = Engine::GetInstance().GetNetworkInfo();

    Color background_color = Color{135, 206, 235, 255};
    Engine::GetInstance().SetBackgroundColor(background_color);
    Engine::GetInstance().SetTitle(game_title);

    window_size = GetWindowSize();

    std::vector<Entity *> entities = CreateEntities();
    AddObjectsToEngine(entities);
    Engine::GetInstance().SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    Engine::GetInstance().Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");

    // Add Game Cleanup code (deallocating pointers)
    DestroyEntities(entities);
    return 0;
}