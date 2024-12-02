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

struct CannonEvent {
    bool move_left;
    bool move_right;
    bool shoot;
} cannon_event;

void Update(std::vector<Entity *> &entities) {}

void UpdateBullet(Entity &bullet) { bullet.GetComponent<Physics>()->SetVelocity({0, -10}); }

void UpdateAlien(Entity &alien) { alien.GetComponent<Physics>()->SetVelocity({0, 0}); }

void HandleBulletEvent(Entity &bullet, Event &event) {
    CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));

    if (collision_event) {
        if (collision_event->collider_1 == &bullet || collision_event->collider_2 == &bullet) {
            if (collision_event->collider_1->GetName() == "alien" ||
                collision_event->collider_2->GetName() == "alien") {
                Log(LogLevel::Info,
                    "Collision between an alien and a bullet detected in HandleBulletEvent");
                Engine::GetInstance().RemoveEntity(collision_event->collider_1);
                Engine::GetInstance().RemoveEntity(collision_event->collider_2);
            }
        }
    }
}

void HandleAlienEvent(Entity &alien, Event &event) {
    CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));

    if (collision_event) {
        if (collision_event->collider_1 == &alien || collision_event->collider_2 == &alien) {
            if (collision_event->collider_1->GetName() == "bullet" ||
                collision_event->collider_2->GetName() == "bullet") {
                Log(LogLevel::Info,
                    "Collision between an alien and a bullet detected in HandleAlienEvent");
                Engine::GetInstance().RemoveEntity(collision_event->collider_1);
                Engine::GetInstance().RemoveEntity(collision_event->collider_2);
            }
        }
    }
}

void Shoot(Position cannon_position) {
    Entity *bullet = new Entity("bullet", EntityCategory::Moving);
    bullet->AddComponent<Transform>();
    bullet->AddComponent<Physics>();
    bullet->AddComponent<Render>();
    bullet->AddComponent<Collision>();
    bullet->AddComponent<Network>();
    bullet->AddComponent<Handler>();

    bullet->GetComponent<Transform>()->SetPosition(
        {cannon_position.x + 100, cannon_position.y - 20});
    bullet->GetComponent<Transform>()->SetSize({3, 10});
    bullet->GetComponent<Render>()->SetColor({255, 255, 0, 255});
    bullet->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    bullet->GetComponent<Handler>()->SetUpdateCallback(UpdateBullet);
    bullet->GetComponent<Handler>()->SetEventCallback(HandleBulletEvent);

    Engine::GetInstance().AddEntity(bullet);
}

void UpdateCannon(Entity &cannon) {
    if (cannon_event.move_right) {
        cannon.GetComponent<Physics>()->SetVelocity(
            {std::min(cannon.GetComponent<Physics>()->GetVelocity().x + 10, 10.0f), 0});
    } else if (cannon_event.move_left) {
        cannon.GetComponent<Physics>()->SetVelocity(
            {std::max(cannon.GetComponent<Physics>()->GetVelocity().x - 10, -10.0f), 0});
    } else {
        cannon.GetComponent<Physics>()->SetVelocity({0, 0});
    }

    if (cannon_event.shoot) {
        // Shoot if the last bullet was fired 100ms ago
        if (Engine::GetInstance().EngineTimelineGetFrameTime().current - last_bullet_fired_time >
            100000000) {
            Shoot(cannon.GetComponent<Transform>()->GetPosition());
            last_bullet_fired_time = Engine::GetInstance().EngineTimelineGetFrameTime().current;
        }
    }
}

// Pass a nullptr if the input event was not a chord event so that the chord actions are disabled.
void HandleCannonChordInput(InputEvent *event) {}

void HandleCannonSingleInput(Entity &cannon, InputEvent *event) {
    HandleCannonChordInput(nullptr);
    bool pressed = event->pressed;
    SDL_Scancode key = event->key;

    switch (key) {
    case SDL_SCANCODE_LEFT:
        cannon_event.move_left = pressed;
        break;
    case SDL_SCANCODE_RIGHT:
        cannon_event.move_right = pressed;
        break;
    case SDL_SCANCODE_SPACE:
        cannon_event.shoot = pressed;
        break;

    default:
        break;
    }
}

void HandleCannonEvent(Entity &cannon, Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));

    if (input_event) {
        switch (input_event->type) {
        case InputEventType::Single:
            HandleCannonSingleInput(cannon, input_event);
            break;
        case InputEventType::Chord:
            HandleCannonChordInput(input_event);
            break;
        default:
            break;
        }
    }
}

Entity *CreateCannon() {
    Entity *cannon = new Entity("cannon", EntityCategory::Controllable);
    cannon->AddComponent<Transform>();
    cannon->AddComponent<Physics>();
    cannon->AddComponent<Render>();
    cannon->AddComponent<Collision>();
    cannon->AddComponent<Network>();
    cannon->AddComponent<Handler>();

    cannon->GetComponent<Transform>()->SetSize({200, 100});
    cannon->GetComponent<Render>()->SetTextureTemplate("cannon_{}.png");
    cannon->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    cannon->GetComponent<Handler>()->SetUpdateCallback(UpdateCannon);
    cannon->GetComponent<Handler>()->SetEventCallback(HandleCannonEvent);

    return cannon;
}

Entity *CreateAlien() {
    Entity *alien = new Entity("alien", EntityCategory::Moving);
    alien->AddComponent<Transform>();
    alien->AddComponent<Physics>();
    alien->AddComponent<Render>();
    alien->AddComponent<Collision>();
    alien->AddComponent<Network>();
    alien->AddComponent<Handler>();

    alien->GetComponent<Transform>()->SetPosition({900, 300});
    alien->GetComponent<Transform>()->SetSize({200, 100});
    alien->GetComponent<Render>()->SetTexture("alien_row_1.png");
    alien->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    alien->GetComponent<Handler>()->SetUpdateCallback(UpdateAlien);
    alien->GetComponent<Handler>()->SetEventCallback(HandleAlienEvent);

    return alien;
}

void CreateSpawnPoints() {
    Engine::GetInstance().AddSpawnPoint(Position{390, 850}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{780, 850}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{1170, 850}, Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{1560, 850}, Size{10, 10});
}

std::vector<Entity *> CreateEntities() {
    std::vector<Entity *> entities;

    // CreateSideBoundaries();
    CreateSpawnPoints();
    // CreateDeathZones();
    Entity *cannon = CreateCannon();
    Entity *alien = CreateAlien();

    entities.push_back(cannon);
    entities.push_back(alien);
    // entities.insert(entities.end(), ground.begin(), ground.end());
    // entities.insert(entities.end(), platforms.begin(), platforms.end());

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