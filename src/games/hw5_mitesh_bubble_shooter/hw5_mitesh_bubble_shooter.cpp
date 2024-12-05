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
const int ROWS = 2, COLUMNS = 27, BUBBLE_SIZE = 70;
const float TIME_TO_MOVE_DOWN = 15.0f;
std::chrono::steady_clock::time_point last_move_down = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point last_update_time = std::chrono::steady_clock::now();
std::vector<Entity *> bubbles;
int64_t last_bullet_fired_time = 0;

struct GunEvent {
    bool move_left;
    bool move_right;
    bool space;
} gun_event;

// Move all bubbles down periodically
void MoveBubblesDown() {
    auto now = std::chrono::steady_clock::now();
    float elapsed_time = std::chrono::duration<float>(now - last_move_down).count();

    if (elapsed_time >= TIME_TO_MOVE_DOWN) {
        for (Entity *bubble : bubbles) {
            Transform *transform = bubble->GetComponent<Transform>();
            Position current_pos = transform->GetPosition();
            transform->SetPosition({current_pos.x, current_pos.y + BUBBLE_SIZE});

            // Check if the bubble touches or crosses the bottom of the screen
            if (current_pos.y + BUBBLE_SIZE >= window_size.height) {
                Log(LogLevel::Info, "Ball went out of the boundary");
                app->quit.store(true);
            }
        }

        last_move_down = now; // Reset timer
    }
}

void Update(std::vector<Entity *> &entities) { MoveBubblesDown(); }

void UpdateBullet(Entity &bullet) { bullet.GetComponent<Physics>()->SetVelocity({10, -10}); }

void UpdateBubble(Entity &bubble) { bubble.GetComponent<Physics>()->SetVelocity({0, 0}); }

void HandleBulletEvent(Entity &bullet, Event &event) {
    CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));

    if (collision_event) {
        if (collision_event->collider_1 == &bullet || collision_event->collider_2 == &bullet) {
            if (collision_event->collider_1->GetName() == "bubble" ||
                collision_event->collider_2->GetName() == "bubble") {
                Log(LogLevel::Info,
                    "Collision between an bubble and a bullet detected in HandleBulletEvent");
                Engine::GetInstance().RemoveEntity(collision_event->collider_1);
                Engine::GetInstance().RemoveEntity(collision_event->collider_2);
            }
        }
    }
}

void HandleBubbleEvent(Entity &bubble, Event &event) {
    CollisionEvent *collision_event = std::get_if<CollisionEvent>(&(event.data));

    if (collision_event) {
        if (collision_event->collider_1 == &bubble || collision_event->collider_2 == &bubble) {
            if (collision_event->collider_1->GetName() == "bullet" ||
                collision_event->collider_2->GetName() == "bullet") {
                Log(LogLevel::Info,
                    "Collision between an bubble and a bullet detected in HandleBulletEvent");
                Engine::GetInstance().RemoveEntity(collision_event->collider_1);
                Engine::GetInstance().RemoveEntity(collision_event->collider_2);
            }
        }
    }
}

// Helper to calculate grid position
Position GetNearestGridPosition(Position pos) {
    float x_coord = std::round(pos.x / BUBBLE_SIZE) * BUBBLE_SIZE;
    float y_coord = std::round(pos.y / BUBBLE_SIZE) * BUBBLE_SIZE;
    return {x_coord, y_coord};
}

// Attach new bubble to grid
void AttachBubble(Entity *bubble) {
    Transform *transform = bubble->GetComponent<Transform>();
    Position grid_pos = GetNearestGridPosition(transform->GetPosition());
    transform->SetPosition(grid_pos);
    bubble->GetComponent<Physics>()->SetVelocity(Velocity{0, 0});
}

// Check for collisions and attach new bubbles
void HandleCollision(Entity *new_bubble) {
    Transform *new_bubble_transform = new_bubble->GetComponent<Transform>();

    for (Entity *existing_bubble : bubbles) {
        if (existing_bubble == new_bubble)
            continue;

        Collision *collision = existing_bubble->GetComponent<Collision>();
        if (collision) {
            AttachBubble(new_bubble);
            bubbles.push_back(new_bubble); // Add to global bubble container
            return;
        }
    }
}

void CreateBullet(Position gun_position) {
    Entity *bullet = new Entity("bullet", EntityCategory::Moving);
    bullet->AddComponent<Transform>();
    bullet->AddComponent<Render>();
    bullet->AddComponent<Collision>();
    bullet->AddComponent<Physics>();
    bullet->AddComponent<Network>();
    bullet->AddComponent<Handler>();

    bullet->GetComponent<Transform>()->SetPosition({gun_position.x + 100, gun_position.y - 40});
    bullet->GetComponent<Transform>()->SetSize({70, 70}); // Bubble size
    bullet->GetComponent<Render>()->SetColor(Color{0, 0, 0, 255});
    bullet->GetComponent<Render>()->SetTexture("red_bubble.png");
    bullet->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    bullet->GetComponent<Handler>()->SetUpdateCallback(UpdateBullet);
    bullet->GetComponent<Handler>()->SetEventCallback(HandleBulletEvent);
    Engine::GetInstance().AddEntity(bullet);
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

void HandlePlayerChordInput(InputEvent *event) {}

void HandlePlayerSingleInput(Entity &gun, InputEvent *event) {
    HandlePlayerChordInput(nullptr);
    bool pressed = event->pressed;
    SDL_Scancode key = event->key;

    switch (key) {
    case SDL_SCANCODE_LEFT:
        gun_event.move_left = pressed;
        break;
    case SDL_SCANCODE_RIGHT:
        gun_event.move_right = pressed;
        break;
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_SPACE:
        gun_event.space = pressed;
        break;

    default:
        break;
    }
}

void HandleGunEvent(Entity &gun, Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));

    if (input_event) {
        switch (input_event->type) {
        case InputEventType::Single:
            HandlePlayerSingleInput(gun, input_event);
            break;
        case InputEventType::Chord:
            HandlePlayerChordInput(input_event);
            break;
        default:
            break;
        }
    }
}

void UpdateGun(Entity &gun) {

    const float TILT_MARGIN = 10.0f;

    // Get current position of the gun
    auto current_position = gun.GetComponent<Transform>()->GetPosition();

    // Process input events
    if (gun_event.move_right) {
        // Tilt the gun to the right by adjusting the position
        gun.GetComponent<Transform>()->SetPosition({current_position.x, current_position.y});
    } else if (gun_event.move_left) {
        // Tilt the gun to the left by adjusting the position
        gun.GetComponent<Transform>()->SetPosition({current_position.x, current_position.y});
    } else {
        // Reset gun position when no movement keys are pressed
        gun.GetComponent<Transform>()->SetPosition({current_position.x, current_position.y});
    }

    if (gun_event.space) {
        // Shoot if the last bullet was fired 100ms ago
        if (Engine::GetInstance().EngineTimelineGetFrameTime().current - last_bullet_fired_time >
            100000000) {
            CreateBullet(gun.GetComponent<Transform>()->GetPosition());
            last_bullet_fired_time = Engine::GetInstance().EngineTimelineGetFrameTime().current;
        }
    }
}

void CreateBubbles(float x_coord, float y_coord, Color color, std::string texture) {
    Entity *bubble = new Entity("bubble", EntityCategory::Stationary);
    bubble->AddComponent<Transform>();
    bubble->AddComponent<Render>();
    bubble->AddComponent<Collision>();
    bubble->AddComponent<Physics>();
    bubble->AddComponent<Network>();
    bubble->AddComponent<Handler>();

    bubble->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    bubble->GetComponent<Render>()->SetTexture(texture);
    bubble->GetComponent<Transform>()->SetPosition({x_coord, y_coord});
    bubble->GetComponent<Transform>()->SetSize({70, 70}); // Bubble size
    bubble->GetComponent<Handler>()->SetUpdateCallback(UpdateBubble);
    bubble->GetComponent<Handler>()->SetEventCallback(HandleBubbleEvent);
    bubbles.push_back(bubble); // Add to global bubble container
}

Entity *CreateGun() {
    Entity *gun = new Entity("gun", EntityCategory::Controllable);
    gun->AddComponent<Transform>();
    gun->AddComponent<Render>();
    gun->AddComponent<Collision>();
    gun->AddComponent<Physics>();
    gun->AddComponent<Network>();
    gun->AddComponent<Handler>();

    gun->GetComponent<Transform>()->SetPosition({900, 1000});
    gun->GetComponent<Transform>()->SetSize({120, 30}); // Bubble size
    gun->GetComponent<Render>()->SetColor(Color{34, 37, 98, 255});
    // gun->GetComponent<Render>()->SetTexture("red_bubble.png");
    gun->GetComponent<Network>()->SetOwner(NetworkRole::Client);
    gun->GetComponent<Handler>()->SetUpdateCallback(UpdateGun);
    gun->GetComponent<Handler>()->SetEventCallback(HandleGunEvent);

    return gun;
}

std::vector<Entity *> CreateEntities() {
    std::vector<Entity *> entities;

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLUMNS; ++col) {
            float x_coord = col * BUBBLE_SIZE;
            float y_coord = row * BUBBLE_SIZE;
            Color color = (row % 2 == 0) ? Color{255, 0, 0, 255}  // Red
                                         : Color{0, 0, 255, 255}; // Blue
            std::string texture = (row % 2 == 0) ? "red_bubble.png" : "blue_bubble.png";
            CreateBubbles(x_coord, y_coord, color, texture);
        }
    }

    entities.insert(entities.end(), bubbles.begin(), bubbles.end());

    Entity *gun = CreateGun();

    entities.push_back(gun);

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
    std::string game_title = "Mitesh's CSC581 HW5 Game: Bubble Shooter";
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