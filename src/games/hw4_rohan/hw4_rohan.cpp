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

void UpdatePlayer(Entity &player) {
    bool player_moved = false;

    if (player_event.move_right) {
        player_moved = true;
        player.GetComponent<Physics>()->SetVelocity(
            {30, player.GetComponent<Physics>()->GetVelocity().y});
    }
    if (player_event.move_left) {
        player_moved = true;
        player.GetComponent<Physics>()->SetVelocity(
            {-30, player.GetComponent<Physics>()->GetVelocity().y});
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

void UpdatePlatforms(Entity &platform) {
    Position platform_position = platform.GetComponent<Transform>()->GetPosition();
    Velocity platform_velocity = platform.GetComponent<Physics>()->GetVelocity();
    float platform_velocity_val = 20;

    if (platform.GetName() == "moving_platform_1") {
        if (platform_position.y < 100) {
            platform.GetComponent<Physics>()->SetVelocity({0, platform_velocity_val});
        } else if (platform_position.y > 900) {
            platform.GetComponent<Physics>()->SetVelocity({0, -platform_velocity_val});
        }

    } else if (platform.GetName() == "moving_platform_2") {
        if (platform_position.x < 2100) {
            platform.GetComponent<Physics>()->SetVelocity({platform_velocity_val, 0});
        } else if (platform_position.x > 2650) {
            platform.GetComponent<Physics>()->SetVelocity({-platform_velocity_val, 0});
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
    moving_platform_1->GetComponent<Handler>()->SetUpdateCallback(UpdatePlatforms);
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
    moving_platform_2->GetComponent<Handler>()->SetUpdateCallback(UpdatePlatforms);
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

    player->GetComponent<Transform>()->SetSize({50, 100});

    player->GetComponent<Physics>()->SetAcceleration({0, 10});
    player->GetComponent<Render>()->SetTextureTemplate("assets/player_{}.png");
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
    Engine::GetInstance().AddDeathZone(Position{-500, -1000}, Size{20, 3000});
    Engine::GetInstance().AddDeathZone(Position{3700, -1000}, Size{20, 3000});
    Engine::GetInstance().AddDeathZone(Position{-500, float(window_size.height)}, Size{4000, 20});
}

void CreateSpawnPoints() {
    Engine::GetInstance().AddSpawnPoint(Position{300, 300}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{400, 250}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{500, 200}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{600, 150}, Size{10, 10});
}

// std::vector<Entity *> CreateFire() {}

std::vector<Entity *> CreateEntities() {
    std::vector<Entity *> entities;

    std::vector<Entity *> ground = CreateGround();
    std::vector<Entity *> platforms = CreatePlatforms();
    CreateSideBoundaries();
    CreateSpawnPoints();
    CreateDeathZones();
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
    std::string game_title = "Rohan's CSC581 HW3 Game: Platformer";
    int max_player_count = 100, texture_count = 4;

    // Initializing the Game Engine
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
    if (network_info.id > max_player_count) {
        Log(LogLevel::Error, "More than %d players spotted: EXITING THE GAME. Player ID: %d",
            max_player_count, network_info.id);
        exit(0);
    }

    Color background_color = Color{165, 200, 255, 255};
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