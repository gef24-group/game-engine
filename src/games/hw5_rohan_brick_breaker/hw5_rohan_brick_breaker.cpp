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
#include <variant>
#include <vector>

NetworkInfo network_info;
Size window_size;
int64_t last_bullet_fired_time = 0;
bool alien_hit_right_boundary = false;
bool alien_hit_left_boundary = false;

struct PaddleEvent {
    bool move_left;
    bool move_right;
} paddle_event;

void Update(std::vector<Entity *> &entities) {
    std::vector<Entity *> bricks;
    bool brick_found = false;
    for (Entity *entity : entities) {
        if (entity->GetName().substr(0, 5) == "brick") {
            brick_found = true;
            break;
        }
    }

    if (!brick_found) {
        Log(LogLevel::Info, "\n\nAll bricks are destroyed. You win!\n\n");
        app->quit.store(true);
    }
}

void UpdatePaddle(Entity &paddle) {
    if (paddle_event.move_right) {
        paddle.GetComponent<Physics>()->SetVelocity(
            {std::min(paddle.GetComponent<Physics>()->GetVelocity().x + 20, 20.0f), 0});
    } else if (paddle_event.move_left) {
        paddle.GetComponent<Physics>()->SetVelocity(
            {std::max(paddle.GetComponent<Physics>()->GetVelocity().x - 20, -20.0f), 0});
    } else {
        paddle.GetComponent<Physics>()->SetVelocity({0, 0});
    }
}

void HandleBrickEvent(Entity &brick, Event &event) {

    CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));

    if (collision_event) {
        if (collision_event->collider_1 == &brick || collision_event->collider_2 == &brick) {
            Color brick_color = brick.GetComponent<Render>()->GetColor();
            if (brick_color.alpha == 255) {
                brick.GetComponent<Render>()->SetColor(
                    {brick_color.red, brick_color.green, brick_color.blue, 125});
            } else if (brick_color.alpha == 125) {
                Engine::GetInstance().RemoveEntity(&brick);
            }
        }
    }
}

void HandleBallEvent(Entity &ball, Event &event) {
    CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));

    if (collision_event) {
        if (collision_event->collider_1 == &ball || collision_event->collider_2 == &ball) {
            if (collision_event->collider_1->GetName() == "bottom_boundary" ||
                collision_event->collider_2->GetName() == "bottom_boundary") {
                Log(LogLevel::Info, "\n\nYou missed the ball :( ! You lose!\n\n");
                app->quit.store(true);
            }
        }
    }
}

// Pass a nullptr if the input event was not a chord event so that the chord actions are disabled.
void HandlePaddleChordInput(InputEvent *event) {}

void HandlePaddleSingleInput(Entity &cannon, InputEvent *event) {
    HandlePaddleChordInput(nullptr);
    bool pressed = event->pressed;
    SDL_Scancode key = event->key;

    switch (key) {
    case SDL_SCANCODE_LEFT:
        paddle_event.move_left = pressed;
        break;
    case SDL_SCANCODE_RIGHT:
        paddle_event.move_right = pressed;
        break;
    default:
        break;
    }
}

void HandlePaddleEvent(Entity &cannon, Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));

    if (input_event) {
        switch (input_event->type) {
        case InputEventType::Single:
            HandlePaddleSingleInput(cannon, input_event);
            break;
        case InputEventType::Chord:
            HandlePaddleChordInput(input_event);
            break;
        default:
            break;
        }
    }
}

Entity *CreatePaddle() {
    Entity *paddle = new Entity("paddle", EntityCategory::Controllable);
    paddle->AddComponent<Transform>();
    paddle->AddComponent<Physics>();
    paddle->AddComponent<Render>();
    paddle->AddComponent<Collision>();
    paddle->AddComponent<Network>();
    paddle->AddComponent<Handler>();

    paddle->GetComponent<Transform>()->SetSize({200, 30});
    paddle->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    paddle->GetComponent<Handler>()->SetUpdateCallback(UpdatePaddle);
    paddle->GetComponent<Handler>()->SetEventCallback(HandlePaddleEvent);

    return paddle;
}

std::vector<Entity *> CreateBricks() {
    std::vector<Entity *> bricks;

    for (int i = 0; i < 10; i++) {
        Entity *brick = new Entity("brick_" + std::to_string(i), EntityCategory::Stationary);
        brick->AddComponent<Transform>();
        brick->AddComponent<Render>();
        brick->AddComponent<Collision>();
        brick->AddComponent<Network>();
        brick->AddComponent<Handler>();

        brick->GetComponent<Transform>()->SetPosition({float(190 + 160 * i), 150});
        brick->GetComponent<Transform>()->SetSize({150, 50});
        brick->GetComponent<Network>()->SetOwner(NetworkRole::Client);
        brick->GetComponent<Render>()->SetColor({85, 0, 0, 125});
        brick->GetComponent<Handler>()->SetEventCallback(HandleBrickEvent);
        bricks.push_back(brick);
    }

    for (int i = 10; i < 20; i++) {
        Entity *brick = new Entity("brick_" + std::to_string(i), EntityCategory::Stationary);
        brick->AddComponent<Transform>();
        brick->AddComponent<Physics>();
        brick->AddComponent<Render>();
        brick->AddComponent<Collision>();
        brick->AddComponent<Network>();
        brick->AddComponent<Handler>();

        brick->GetComponent<Transform>()->SetPosition({float(190 + 160 * (i - 10)), 80});
        brick->GetComponent<Transform>()->SetSize({150, 50});
        brick->GetComponent<Network>()->SetOwner(NetworkRole::Client);
        brick->GetComponent<Render>()->SetColor({85, 0, 0, 255});
        brick->GetComponent<Handler>()->SetEventCallback(HandleBrickEvent);
        bricks.push_back(brick);
    }

    return bricks;
}

Entity *CreateBall() {
    Entity *ball = new Entity("ball", EntityCategory::Moving);
    ball->AddComponent<Transform>();
    ball->AddComponent<Physics>();
    ball->AddComponent<Collision>();
    ball->AddComponent<Network>();
    ball->AddComponent<Render>();
    ball->AddComponent<Handler>();

    ball->GetComponent<Transform>()->SetPosition({600, 500});
    ball->GetComponent<Transform>()->SetSize({50, 50});
    ball->GetComponent<Physics>()->SetVelocity({10, 10});
    ball->GetComponent<Collision>()->SetRestitution(1);
    ball->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    ball->GetComponent<Render>()->SetTexture("ball.png");
    ball->GetComponent<Handler>()->SetEventCallback(HandleBallEvent);

    return ball;
}

std::vector<Entity *> CreateGameBoundaries() {
    std::vector<Entity *> game_boundaries;

    Entity *left_boundary = new Entity("left_boundary", EntityCategory::Stationary);
    Entity *right_boundary = new Entity("right_boundary", EntityCategory::Stationary);
    Entity *top_boundary = new Entity("top_boundary", EntityCategory::Stationary);
    Entity *bottom_boundary = new Entity("bottom_boundary", EntityCategory::Stationary);

    left_boundary->AddComponent<Transform>();
    left_boundary->AddComponent<Collision>();
    left_boundary->AddComponent<Render>();

    right_boundary->AddComponent<Transform>();
    right_boundary->AddComponent<Collision>();
    right_boundary->AddComponent<Render>();

    top_boundary->AddComponent<Transform>();
    top_boundary->AddComponent<Collision>();
    top_boundary->AddComponent<Render>();

    bottom_boundary->AddComponent<Transform>();
    bottom_boundary->AddComponent<Collision>();
    bottom_boundary->AddComponent<Render>();

    left_boundary->GetComponent<Transform>()->SetPosition({0, 0});
    left_boundary->GetComponent<Transform>()->SetSize({50, window_size.height});
    left_boundary->GetComponent<Render>()->SetColor({192, 192, 192, 255});

    right_boundary->GetComponent<Transform>()->SetPosition({float(window_size.width - 50), 0});
    right_boundary->GetComponent<Transform>()->SetSize({50, window_size.height});
    right_boundary->GetComponent<Render>()->SetColor({192, 192, 192, 255});

    top_boundary->GetComponent<Transform>()->SetPosition({50, 0});
    top_boundary->GetComponent<Transform>()->SetSize({window_size.width - 100, 50});
    top_boundary->GetComponent<Render>()->SetColor({192, 192, 192, 255});

    bottom_boundary->GetComponent<Transform>()->SetPosition({50, float(window_size.height - 50)});
    bottom_boundary->GetComponent<Transform>()->SetSize({window_size.width - 100, 50});
    bottom_boundary->GetComponent<Render>()->SetColor({255, 0, 0, 255});

    game_boundaries.push_back(left_boundary);
    game_boundaries.push_back(right_boundary);
    game_boundaries.push_back(top_boundary);
    game_boundaries.push_back(bottom_boundary);

    return game_boundaries;
}

void CreateSpawnPoints() {
    Engine::GetInstance().AddSpawnPoint(Position{390, 850}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{780, 850}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{1170, 850}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{1560, 850}, Size{10, 10});
}

std::vector<Entity *> CreateEntities() {
    std::vector<Entity *> entities;

    CreateSpawnPoints();
    Entity *paddle = CreatePaddle();
    std::vector<Entity *> bricks = CreateBricks();
    Entity *ball = CreateBall();
    std::vector<Entity *> game_boundaries = CreateGameBoundaries();

    entities.push_back(paddle);
    entities.push_back(ball);
    entities.insert(entities.end(), bricks.begin(), bricks.end());
    entities.insert(entities.end(), game_boundaries.begin(), game_boundaries.end());

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
    std::string game_title = "Rohan's CSC581 HW5 Game: Brick Breaker";
    int max_player_count = 100, texture_count = 1;

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
    Engine::GetInstance().SetShowPlayerBorder(false);

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
