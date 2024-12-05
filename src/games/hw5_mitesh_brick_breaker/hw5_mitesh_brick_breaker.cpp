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

struct GunEvent {
    bool move_left;
    bool move_right;
} platform_event;

void Update(std::vector<Entity *> &entities) {
    bool brick_found = false;
    for (Entity *entity : entities) {
        if (entity->GetName().substr(0, 5) == "brick") {
            brick_found = true;
            break;
        }
    }

    if (!brick_found) {
        Log(LogLevel::Info, "All bricks are destroyed. You win!");
        app->quit.store(true);
    }
}

void UpdatePlatform(Entity &platform) {
    if (platform_event.move_right) {
        platform.GetComponent<Physics>()->SetVelocity(
            {std::min(platform.GetComponent<Physics>()->GetVelocity().x + 50, 50.0f), 0});
    } else if (platform_event.move_left) {
        platform.GetComponent<Physics>()->SetVelocity(
            {std::max(platform.GetComponent<Physics>()->GetVelocity().x - 50, -50.0f), 0});
    } else {
        platform.GetComponent<Physics>()->SetVelocity({0, 0});
    }
}

void HandleBrickEvent(Entity &brick, Event &event) {
    CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));

    if (collision_event) {
        if (collision_event->collider_1 == &brick || collision_event->collider_2 == &brick) {
            Engine::GetInstance().RemoveEntity(&brick);
        }
    }
}

void HandleBallEvent(Entity &ball, Event &event) {
    CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));

    float ball_x = ball.GetComponent<Transform>()->GetPosition().x;
    float ball_y = ball.GetComponent<Transform>()->GetPosition().y;

    if (ball_y + ball.GetComponent<Transform>()->GetSize().height > window_size.height) {
        Log(LogLevel::Info, "Ball went out of the boundary");
        app->quit.store(true);
    }
}

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

void HandlePlatformChordInput(InputEvent *event) {}

void HandlePlatformSingleInput(Entity &gun, InputEvent *event) {
    HandlePlatformChordInput(nullptr);
    bool pressed = event->pressed;
    SDL_Scancode key = event->key;

    switch (key) {
    case SDL_SCANCODE_LEFT:
        platform_event.move_left = pressed;
        break;
    case SDL_SCANCODE_RIGHT:
        platform_event.move_right = pressed;
        break;
    default:
        break;
    }
}

void HandlePlatformEvent(Entity &platform, Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));

    if (input_event) {
        switch (input_event->type) {
        case InputEventType::Single:
            HandlePlatformSingleInput(platform, input_event);
            break;
        case InputEventType::Chord:
            HandlePlatformChordInput(input_event);
            break;
        default:
            break;
        }
    }
}

std::vector<Entity *> CreateWalls() {
    std::vector<Entity *> walls;

    Entity *left_wall = new Entity("left_wall", EntityCategory::Stationary);
    Entity *right_wall = new Entity("right_wall", EntityCategory::Stationary);
    Entity *top_wall = new Entity("top_wall", EntityCategory::Stationary);

    left_wall->AddComponent<Transform>();
    left_wall->AddComponent<Collision>();
    left_wall->AddComponent<Render>();

    right_wall->AddComponent<Transform>();
    right_wall->AddComponent<Collision>();
    right_wall->AddComponent<Render>();

    top_wall->AddComponent<Transform>();
    top_wall->AddComponent<Collision>();
    top_wall->AddComponent<Render>();

    left_wall->GetComponent<Transform>()->SetPosition({0, 0});
    left_wall->GetComponent<Transform>()->SetSize({20, window_size.height});
    left_wall->GetComponent<Render>()->SetColor({255, 131, 169, 255});

    right_wall->GetComponent<Transform>()->SetPosition({float(window_size.width - 20), 0});
    right_wall->GetComponent<Transform>()->SetSize({50, window_size.height});
    right_wall->GetComponent<Render>()->SetColor({255, 131, 169, 255});

    top_wall->GetComponent<Transform>()->SetPosition({20, 0});
    top_wall->GetComponent<Transform>()->SetSize({window_size.width - 100, 20});
    top_wall->GetComponent<Render>()->SetColor({255, 131, 169, 255});

    walls.push_back(left_wall);
    walls.push_back(right_wall);
    walls.push_back(top_wall);

    return walls;
}

Entity *CreateBall() {
    Entity *ball = new Entity("ball", EntityCategory::Moving);
    ball->AddComponent<Transform>();
    ball->AddComponent<Physics>();
    ball->AddComponent<Collision>();
    ball->AddComponent<Network>();
    ball->AddComponent<Render>();
    ball->AddComponent<Handler>();

    ball->GetComponent<Transform>()->SetPosition({200, 500});
    ball->GetComponent<Transform>()->SetSize({50, 50});
    ball->GetComponent<Physics>()->SetVelocity({40, 40});
    ball->GetComponent<Collision>()->SetRestitution(1);
    ball->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    ball->GetComponent<Render>()->SetTexture("ball.png");
    ball->GetComponent<Handler>()->SetEventCallback(HandleBallEvent);

    return ball;
}

std::vector<Entity *> CreateBricks() {

    std::vector<Entity *> bricks;
    int number_of_bricks = 0;

    while (number_of_bricks < 10) {
        Entity *brick =
            new Entity("brick" + std::to_string(number_of_bricks), EntityCategory::Stationary);
        brick->AddComponent<Transform>();
        brick->AddComponent<Render>();
        brick->AddComponent<Collision>();
        brick->AddComponent<Network>();
        brick->AddComponent<Handler>();

        brick->GetComponent<Transform>()->SetPosition({float(190 + 160 * number_of_bricks), 220});
        brick->GetComponent<Transform>()->SetSize({150, 50});
        brick->GetComponent<Render>()->SetColor({255, 0, 0, 255});
        brick->GetComponent<Render>()->SetTexture("brick.png");
        brick->GetComponent<Network>()->SetOwner(NetworkRole::Client);
        brick->GetComponent<Handler>()->SetEventCallback(HandleBrickEvent);
        bricks.push_back(brick);
        number_of_bricks++;
    }

    while (number_of_bricks < 20) {
        Entity *brick =
            new Entity("brick" + std::to_string(number_of_bricks), EntityCategory::Stationary);
        brick->AddComponent<Transform>();
        brick->AddComponent<Render>();
        brick->AddComponent<Collision>();
        brick->AddComponent<Network>();
        brick->AddComponent<Handler>();

        brick->GetComponent<Transform>()->SetPosition(
            {float(190 + 160 * (number_of_bricks - 10)), 150});
        brick->GetComponent<Transform>()->SetSize({150, 50});
        brick->GetComponent<Render>()->SetColor({255, 0, 0, 255});
        brick->GetComponent<Render>()->SetTexture("brick.png");
        brick->GetComponent<Network>()->SetOwner(NetworkRole::Client);
        brick->GetComponent<Handler>()->SetEventCallback(HandleBrickEvent);
        bricks.push_back(brick);
        number_of_bricks++;
    }

    while (number_of_bricks < 30) {
        Entity *brick =
            new Entity("brick" + std::to_string(number_of_bricks), EntityCategory::Stationary);
        brick->AddComponent<Transform>();
        brick->AddComponent<Render>();
        brick->AddComponent<Collision>();
        brick->AddComponent<Network>();
        brick->AddComponent<Handler>();

        brick->GetComponent<Transform>()->SetPosition(
            {float(190 + 160 * (number_of_bricks - 20)), 80});
        brick->GetComponent<Transform>()->SetSize({150, 50});
        brick->GetComponent<Render>()->SetColor({255, 0, 0, 255});
        brick->GetComponent<Render>()->SetTexture("brick.png");
        brick->GetComponent<Network>()->SetOwner(NetworkRole::Client);
        brick->GetComponent<Handler>()->SetEventCallback(HandleBrickEvent);
        bricks.push_back(brick);
        number_of_bricks++;
    }

    return bricks;
}

Entity *CreatePlatform() {
    Entity *platform = new Entity("platform", EntityCategory::Controllable);
    platform->AddComponent<Transform>();
    platform->AddComponent<Render>();
    platform->AddComponent<Collision>();
    platform->AddComponent<Physics>();
    platform->AddComponent<Network>();
    platform->AddComponent<Handler>();

    platform->GetComponent<Transform>()->SetPosition({700, 1000});
    platform->GetComponent<Transform>()->SetSize({250, 25}); // Bubble size
    // gun->GetComponent<Render>()->SetColor(Color{0, 0, 0, 255});
    // platform->GetComponent<Render>()->SetTexture("red_bubble.png");
    platform->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    platform->GetComponent<Handler>()->SetUpdateCallback(UpdatePlatform);
    platform->GetComponent<Handler>()->SetEventCallback(HandlePlatformEvent);

    return platform;
}

std::vector<Entity *> CreateEntities() {
    std::vector<Entity *> entities;

    // CreateSpawnPoints();
    Entity *platform = CreatePlatform();
    std::vector<Entity *> bricks = CreateBricks();
    Entity *ball = CreateBall();
    std::vector<Entity *> walls = CreateWalls();

    entities.push_back(platform);
    entities.push_back(ball);
    entities.insert(entities.end(), bricks.begin(), bricks.end());
    entities.insert(entities.end(), walls.begin(), walls.end());

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
    std::string game_title = "Mitesh's CSC581 HW5 Game: Brick Breaker";
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
    Engine::GetInstance().SetShowPlayerBorder(false);

    network_info = Engine::GetInstance().GetNetworkInfo();

    Color background_color = Color{212, 248, 253, 255};
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