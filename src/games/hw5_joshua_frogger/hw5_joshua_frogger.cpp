#include "Collision.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "EventManager.hpp"
#include "Handler.hpp"
#include "Network.hpp"
#include "Physics.hpp"
#include "Render.hpp"
#include "SDL_scancode.h"
#include "Transform.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <string>
#include <unordered_set>

const int TILE_SIZE = 75;
const int ROWS = 14;
const int COLUMNS = 14;
int filled_homes = 0;
Position tile_0 = Position{0, 0};
Color background_color = Color{33, 37, 43, 255};
Color river_color = Color{41, 41, 182, 255};
std::vector<Entity *> river_bodies;

Size window_size;
NetworkInfo network_info;

void Update(std::vector<Entity *> &entities) {}

void SetRiverBodies(std::vector<Entity *> &entities) {
    for (const auto &entity : entities) {
        if (entity->GetName().find("bush") == 0 || entity->GetName().find("home") == 0 ||
            entity->GetName().find("turtle") == 0 || entity->GetName().find("log") == 0) {
            river_bodies.push_back(entity);
        }
    }
}

bool IsSignificantOverlap(Entity &entity_1, Entity &entity_2) {
    float x_1 = entity_1.GetComponent<Transform>()->GetPosition().x;
    float w_1 = float(entity_1.GetComponent<Transform>()->GetSize().width);
    float x_2 = entity_2.GetComponent<Transform>()->GetPosition().x;
    float w_2 = float(entity_2.GetComponent<Transform>()->GetSize().width);

    float threshold = 0.6;
    float overlap_width = std::max(0.0f, std::min(x_1 + w_1, x_2 + w_2) - std::max(x_1, x_2));

    return (overlap_width / w_1) >= threshold;
}

Entity *GetOverlappingRiverBody(Entity &frog, Position &position) {
    Entity *overlapping_river_body = nullptr;

    for (const auto &entity : river_bodies) {
        if (entity->GetComponent<Transform>()->GetPosition().y == position.y) {
            if (IsSignificantOverlap(frog, *entity)) {
                overlapping_river_body = entity;
                break;
            }
        }
    }

    return overlapping_river_body;
}

void MoveFrog(Entity &frog, Position &position) {
    if (position.y < tile_0.y - (TILE_SIZE * 12)) {
        EventManager::GetInstance().RaiseDeathEvent(DeathEvent{&frog});
    }
    EventManager::GetInstance().RaiseMoveEvent(MoveEvent{&frog, position});

    if (position.y >= tile_0.y - (TILE_SIZE * 6)) {
        return;
    }

    Entity *overlapping_river_body = GetOverlappingRiverBody(frog, position);
    if (overlapping_river_body == nullptr) {
        EventManager::GetInstance().RaiseDeathEvent(DeathEvent{&frog});
        return;
    }
    if (overlapping_river_body->GetName().find("bush") == 0) {
        EventManager::GetInstance().RaiseDeathEvent(DeathEvent{&frog});
        return;
    }

    if ((overlapping_river_body->GetName().find("turtle") == 0) ||
        (overlapping_river_body->GetName().find("log") == 0)) {
        frog.GetComponent<Physics>()->SetVelocity(
            overlapping_river_body->GetComponent<Physics>()->GetVelocity());
    }
    if (overlapping_river_body->GetName().find("home") == 0) {
        if (overlapping_river_body->GetComponent<Render>()->GetTexturePath() == "") {
            filled_homes += 1;
            overlapping_river_body->GetComponent<Render>()->SetTexture("frog_home.png");
        }
        EventManager::GetInstance().RaiseDeathEvent(DeathEvent{&frog});

        if (filled_homes == 5) {
            Log(LogLevel::Info, "");
            Log(LogLevel::Info, "Thank you for taking me home!");
            Log(LogLevel::Info, "");
            app->quit.store(true);
        }
    }
}

void HandleFrogSingleInput(Entity &frog, InputEvent *event) {
    bool pressed = event->pressed;
    if (!pressed) {
        return;
    }

    Position cur_pos = frog.GetComponent<Transform>()->GetPosition();
    Position new_pos = cur_pos;

    SDL_Scancode key = event->key;
    switch (key) {
    case SDL_SCANCODE_W: {
        new_pos.y = cur_pos.y - TILE_SIZE;
        break;
    }
    case SDL_SCANCODE_S: {
        new_pos.y = cur_pos.y + TILE_SIZE;
        break;
    }
    case SDL_SCANCODE_A: {
        new_pos.x = cur_pos.x - TILE_SIZE;
        break;
    }
    case SDL_SCANCODE_D: {
        new_pos.x = cur_pos.x + TILE_SIZE;
        break;
    }
    default:
        break;
    }

    MoveFrog(frog, new_pos);
}

void HandleFrogChordInput(Entity &frog, InputEvent *event) {
    bool pressed = event->pressed;
    if (!pressed) {
        return;
    }

    Position cur_pos = frog.GetComponent<Transform>()->GetPosition();
    Position new_pos = cur_pos;

    switch (event->chord_id) {
    case 1: {
        new_pos.y = cur_pos.y - (TILE_SIZE * 2);
        break;
    }
    case 2: {
        new_pos.y = cur_pos.y + (TILE_SIZE * 2);
        break;
    }
    case 3: {
        new_pos.x = cur_pos.x - (TILE_SIZE * 2);
        break;
    }
    case 4: {
        new_pos.x = cur_pos.x + (TILE_SIZE * 2);
        break;
    }
    default:
        break;
    }

    MoveFrog(frog, new_pos);
}

void HandleFrogInput(Entity &frog, Event &event) {
    InputEvent *input_event = std::get_if<InputEvent>(&(event.data));
    if (input_event == nullptr) {
        return;
    }

    switch (input_event->type) {
    case InputEventType::Single:
        HandleFrogSingleInput(frog, input_event);
        break;
    case InputEventType::Chord:
        HandleFrogChordInput(frog, input_event);
        break;
    default:
        break;
    }
}

void HandleFrogEvent(Entity &frog, Event &event) {
    switch (event.type) {
    case EventType::Input:
        HandleFrogInput(frog, event);
        break;
    default:
        break;
    }
}

void UpdateFrog(Entity &frog) {
    float frog_right_edge = frog.GetComponent<Transform>()->GetPosition().x +
                            float(frog.GetComponent<Transform>()->GetSize().width);
    float frog_left_edge = frog.GetComponent<Transform>()->GetPosition().x;
    float frog_top_edge = frog.GetComponent<Transform>()->GetPosition().y;
    float map_left_edge = tile_0.x;
    float map_right_edge = tile_0.x + (TILE_SIZE * COLUMNS);

    bool frog_out_left = frog_right_edge < map_left_edge;
    bool frog_out_right = frog_left_edge > map_right_edge;

    if (frog_top_edge >= tile_0.y - (TILE_SIZE * 6)) {
        frog.GetComponent<Physics>()->SetVelocity(Velocity{0, 0});
    }

    if (frog_out_left || frog_out_right) {
        EventManager::GetInstance().RaiseDeathEvent(DeathEvent{&frog});
    }
}

void UpdateVehicle(Entity &vehicle) {
    float vehicle_right_edge = vehicle.GetComponent<Transform>()->GetPosition().x +
                               float(vehicle.GetComponent<Transform>()->GetSize().width);
    float vehicle_left_edge = vehicle.GetComponent<Transform>()->GetPosition().x;
    float map_left_edge = tile_0.x;
    float map_right_edge = tile_0.x + (TILE_SIZE * COLUMNS);

    bool vehicle_out_left = vehicle_right_edge < map_left_edge;
    bool vehicle_out_right = vehicle_left_edge > map_right_edge;

    Position new_pos = vehicle.GetComponent<Transform>()->GetPosition();

    if (vehicle_out_left) {
        new_pos.x = map_right_edge;
        EventManager::GetInstance().RaiseMoveEvent(MoveEvent{&vehicle, new_pos});
    }
    if (vehicle_out_right) {
        new_pos.x = map_left_edge - TILE_SIZE;
        EventManager::GetInstance().RaiseMoveEvent(MoveEvent{&vehicle, new_pos});
    }
}

void CreateFrog(std::vector<Entity *> &entities) {
    Entity *frog = new Entity("frog", EntityCategory::Controllable);
    frog->AddComponent<Render>();
    frog->AddComponent<Transform>();
    frog->AddComponent<Physics>();
    frog->AddComponent<Collision>();
    frog->AddComponent<Handler>();
    frog->AddComponent<Network>();

    frog->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
    frog->GetComponent<Render>()->SetTextureTemplate("frog_{}.png");
    frog->GetComponent<Render>()->SetDepth(3);
    frog->GetComponent<Transform>()->SetSize(Size{TILE_SIZE, TILE_SIZE});
    frog->GetComponent<Collision>()->SetAvoidTransform(true);
    frog->GetComponent<Handler>()->SetEventCallback(HandleFrogEvent);
    frog->GetComponent<Handler>()->SetUpdateCallback(UpdateFrog);
    frog->GetComponent<Network>()->SetOwner(NetworkRole::Client);

    entities.push_back(frog);
}

void CreateVehicles(std::vector<Entity *> &entities) {
    int vehicle_index = 0;

    for (int row = 1; row < 6; row++) {
        for (int vehicle_count = 0; vehicle_count < 3; vehicle_count++) {
            std::string vehicle_texture = "car_" + std::to_string(row) + ".png";
            Position vehicle_position = Position{tile_0.x + float(vehicle_count * TILE_SIZE * 5),
                                                 tile_0.y - float(row * TILE_SIZE)};
            Velocity vehicle_velocity = Velocity{10, 0};
            Size vehicle_size = Size{TILE_SIZE, TILE_SIZE};

            if (row == 5) {
                vehicle_texture = "truck.png";
                vehicle_velocity.x *= 0.8;
                vehicle_size.width *= 2;
            }
            if (row % 2 != 0) {
                vehicle_velocity.x *= -1;
            }
            if (row == 1 || row == 2) {
                vehicle_velocity.x *= 1.5;
            }

            Entity *vehicle =
                new Entity("vehicle_" + std::to_string(vehicle_index), EntityCategory::DeathZone);
            vehicle->AddComponent<Transform>();
            vehicle->AddComponent<Physics>();
            vehicle->AddComponent<Handler>();
            vehicle->AddComponent<Render>();
            vehicle->AddComponent<Network>();

            vehicle->GetComponent<Transform>()->SetPosition(vehicle_position);
            vehicle->GetComponent<Transform>()->SetSize(vehicle_size);
            vehicle->GetComponent<Physics>()->SetVelocity(vehicle_velocity);
            vehicle->GetComponent<Handler>()->SetUpdateCallback(UpdateVehicle);
            vehicle->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
            vehicle->GetComponent<Render>()->SetTexture(vehicle_texture);
            vehicle->GetComponent<Render>()->SetDepth(1);
            vehicle->GetComponent<Network>()->SetOwner(NetworkRole::Server);

            vehicle_index++;

            entities.push_back(vehicle);
        }
    }
}

void CreateFloaters(std::vector<Entity *> &entities) {
    int floater_index = 0;

    for (int row = 7; row < 12; row++) {
        for (int floater_count = 0; floater_count < 3; floater_count++) {
            std::string floater_name = "";
            int floater_length = 3;
            std::string floater_texture = "";
            Position floater_position = Position{tile_0.x + float(floater_count * TILE_SIZE * 5),
                                                 tile_0.y - float(row * TILE_SIZE)};
            Velocity floater_velocity = Velocity{10, 0};

            if (row == 7 || row == 10) {
                floater_name = "turtle";
                floater_velocity.x *= -1;
            }
            if (row == 8 || row == 9 || row == 11) {
                floater_name = "log";
                if ((row == 9) && (floater_count < 2)) {
                    floater_length -= 1;
                }
                if ((row == 8 || row == 11) && (floater_count == 0 || floater_count == 2)) {
                    floater_length -= 1;
                }
            }
            if (row == 10) {
                floater_velocity.x *= 2;
            }
            if (row == 7 || row == 8) {
                floater_velocity.x *= 1.5;
            }

            floater_texture = floater_name + "_" + std::to_string(floater_length) + ".png";

            Entity *floater = new Entity(floater_name + "_" + std::to_string(floater_index),
                                         EntityCategory::Moving);
            floater->AddComponent<Transform>();
            floater->AddComponent<Physics>();
            floater->AddComponent<Handler>();
            floater->AddComponent<Render>();
            floater->AddComponent<Network>();

            floater->GetComponent<Transform>()->SetPosition(floater_position);
            floater->GetComponent<Transform>()->SetSize(
                Size{TILE_SIZE * floater_length, TILE_SIZE});
            floater->GetComponent<Physics>()->SetVelocity(floater_velocity);
            floater->GetComponent<Handler>()->SetUpdateCallback(UpdateVehicle);
            floater->GetComponent<Render>()->SetColor(Color{0, 0, 0, 0});
            floater->GetComponent<Render>()->SetTexture(floater_texture);
            floater->GetComponent<Render>()->SetDepth(1);
            floater->GetComponent<Network>()->SetOwner(NetworkRole::Server);

            floater_index++;

            entities.push_back(floater);
        }
    }
}

void CreateMap(std::vector<Entity *> &entities) {
    int tile_index = 0;

    for (int section = 0; section < 6; section++) {
        int sub_sections = 1;

        std::string tile_name = "";
        Position tile_position = Position{0, 0};
        Size tile_size = Size{0, 0};
        Color tile_color = Color{0, 0, 0, 0};
        std::string texture_path = "";

        if (section == 0 || section == 2) {
            tile_name = "pavement";
            if (section == 0) {
                tile_position = Position{tile_0.x, tile_0.y};
            }
            if (section == 2) {
                tile_position = Position{tile_0.x, tile_0.y - (TILE_SIZE * 6)};
            }
            tile_size = Size{TILE_SIZE * COLUMNS, TILE_SIZE};
            texture_path = "pavement.png";
        }
        if (section == 1) {
            tile_name = "road";
            tile_position = Position{tile_0.x, tile_0.y - (TILE_SIZE * 5)};
            tile_size = Size{TILE_SIZE * COLUMNS, TILE_SIZE * 5};
            tile_color = background_color;
        }
        if (section == 3) {
            tile_name = "river";
            tile_position = Position{tile_0.x, tile_0.y - (TILE_SIZE * 11)};
            tile_size = Size{TILE_SIZE * COLUMNS, TILE_SIZE * 5};
            tile_color = river_color;
        }
        if (section == 5) {
            tile_name = "swamp";
            tile_position = Position{tile_0.x, tile_0.y - (TILE_SIZE * 13)};
            tile_size = Size{TILE_SIZE * COLUMNS, TILE_SIZE};
            texture_path = "swamp.png";
        }
        if (section == 4) {
            sub_sections = COLUMNS;
        }

        for (int sub_section_index = 0; sub_section_index < sub_sections; sub_section_index++) {
            if (section == 4) {
                tile_name = "bush";
                tile_position = Position{tile_0.x + float(TILE_SIZE * sub_section_index),
                                         tile_0.y - (TILE_SIZE * 12)};
                tile_size = Size{TILE_SIZE, TILE_SIZE};
                texture_path = "bush.png";
                if (sub_section_index == 2 || sub_section_index == 4 || sub_section_index == 6 ||
                    sub_section_index == 8 || sub_section_index == 10) {
                    tile_name = "home";
                    tile_color = river_color;
                    texture_path = "";
                }
            }

            Entity *tile =
                new Entity(tile_name + "_" + std::to_string(tile_index), EntityCategory::Moving);
            tile->AddComponent<Render>();
            tile->AddComponent<Transform>();
            tile->AddComponent<Network>();

            tile->GetComponent<Transform>()->SetPosition(tile_position);
            tile->GetComponent<Transform>()->SetSize(tile_size);
            tile->GetComponent<Render>()->SetColor(tile_color);
            if (!texture_path.empty()) {
                tile->GetComponent<Render>()->SetTexture(texture_path);
            }
            tile->GetComponent<Network>()->SetOwner(NetworkRole::Client);

            tile_index++;

            entities.push_back(tile);
        }
    }
}

std::vector<Entity *> CreateEntities() {
    std::vector<Entity *> entities = std::vector<Entity *>();

    CreateFrog(entities);
    CreateVehicles(entities);
    CreateFloaters(entities);
    CreateMap(entities);

    for (Entity *entity : entities) {
        if (network_info.mode == NetworkMode::PeerToPeer) {
            if (entity->GetComponent<Network>() == nullptr) {
                continue;
            }

            if (entity->GetComponent<Network>()->GetOwner() == NetworkRole::Server) {
                entity->GetComponent<Network>()->SetOwner(NetworkRole::Host);
            }
            if (entity->GetComponent<Network>()->GetOwner() == NetworkRole::Client) {
                entity->GetComponent<Network>()->SetOwner(NetworkRole::Peer);
            }
        }
    }

    return entities;
}

void CreateSpawnPoints() {
    Engine::GetInstance().AddSpawnPoint(Position{tile_0.x + float(7 * TILE_SIZE), tile_0.y},
                                        Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{tile_0.x + float(3 * TILE_SIZE), tile_0.y},
                                        Size{10, 10});
    Engine::GetInstance().AddSpawnPoint(Position{tile_0.x + float(10 * TILE_SIZE), tile_0.y},
                                        Size{10, 10});
}

void CreateDeathZones() {
    int thickness = TILE_SIZE;

    Engine::GetInstance().AddDeathZone(
        Position{float(tile_0.x - (TILE_SIZE * 1.5)), float(tile_0.y - (TILE_SIZE * 6))},
        Size{thickness, window_size.height * 2});
    Engine::GetInstance().AddDeathZone(
        Position{float(tile_0.x + (TILE_SIZE * COLUMNS) + (TILE_SIZE * 1.5) - thickness),
                 float(tile_0.y - (TILE_SIZE * 6))},
        Size{thickness, window_size.height * 2});
    Engine::GetInstance().AddDeathZone(
        Position{-float(window_size.width),
                 float(tile_0.y - (TILE_SIZE * (ROWS - 1)) - (TILE_SIZE * 1.5))},
        Size{window_size.width * 3, thickness});
    Engine::GetInstance().AddDeathZone(
        Position{-float(window_size.width),
                 float(tile_0.y + TILE_SIZE + (TILE_SIZE * 1.5) - thickness)},
        Size{window_size.width * 3, thickness});
}

void DestroyEntities(std::vector<Entity *> entities) {
    for (Entity *entity : entities) {
        delete entity;
    }
}

void BindEngineInputs() {
    Engine::GetInstance().BindPauseKey(SDL_SCANCODE_P);
    Engine::GetInstance().BindSpeedDownKey(SDL_SCANCODE_COMMA);
    Engine::GetInstance().BindSpeedUpKey(SDL_SCANCODE_PERIOD);
    Engine::GetInstance().BindDisplayScalingKey(SDL_SCANCODE_X);
    Engine::GetInstance().BindHiddenZoneKey(SDL_SCANCODE_Z);
    Engine::GetInstance().BindRecordKey(SDL_SCANCODE_Q);
    Engine::GetInstance().BindReplayKey(SDL_SCANCODE_R);
}

void RegisterInputChords() {
    Engine::GetInstance().RegisterInputChord(1, {SDL_SCANCODE_W, SDL_SCANCODE_P});
    Engine::GetInstance().RegisterInputChord(2, {SDL_SCANCODE_S, SDL_SCANCODE_P});
    Engine::GetInstance().RegisterInputChord(3, {SDL_SCANCODE_A, SDL_SCANCODE_P});
    Engine::GetInstance().RegisterInputChord(4, {SDL_SCANCODE_D, SDL_SCANCODE_P});
}

int main(int argc, char *args[]) {
    std::string title = "CSC581 HW5 Joshua's Frogger";

    if (!SetEngineCLIOptions(argc, args)) {
        return 1;
    }
    network_info = Engine::GetInstance().GetNetworkInfo();

    Engine::GetInstance().SetBackgroundColor(background_color);
    Engine::GetInstance().SetTitle(title);
    Engine::GetInstance().SetShowPlayerBorder(true);

    if (!Engine::GetInstance().Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    BindEngineInputs();
    RegisterInputChords();
    network_info = Engine::GetInstance().GetNetworkInfo();
    window_size = GetWindowSize();
    tile_0.x = float((window_size.width / 2.0) - ((TILE_SIZE * COLUMNS) / 2.0));
    tile_0.y = float((window_size.height / 2.0) + (((ROWS / 2.0) - 1) * TILE_SIZE));

    CreateSpawnPoints();
    CreateDeathZones();

    std::vector<Entity *> entities = CreateEntities();
    SetRiverBodies(entities);

    for (Entity *entity : entities) {
        Engine::GetInstance().AddEntity(entity);
    }

    Engine::GetInstance().SetPlayerTextures(5);
    Engine::GetInstance().SetCallback(Update);

    Engine::GetInstance().Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    DestroyEntities(entities);
    return 0;
}