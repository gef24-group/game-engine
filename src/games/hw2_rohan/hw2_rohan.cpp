#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <atomic>
#include <string>
#include <vector>

// Head soccer
Size window_size;
int team1_score = 0, team2_score = 0;

void Update(std::vector<GameObject *> *game_objects) {
    GameObject *ball = NULL, *player = NULL;
    for (GameObject *object : *game_objects) {
        if (object->GetName() == "ball") {
            ball = object;
        }
        if (object->GetName().find("player") == 0) {
            player = object;
        }
    }

    if (ball == NULL || player == NULL) {
        return;
    }

    Velocity ball_velocity = ball->GetVelocity();
    Position ball_position = ball->GetPosition();
    if (ball_position.x < 0 || ball_position.x > float(window_size.width)) {
        ball->SetVelocity({-ball_velocity.x, ball_velocity.y});
    }

    // If the ball collides with the basket, the player scores a goal
    for (GameObject *object : ball->GetColliders()) {
        if (object->GetName() == "basket_1") {
            // app->quit.store(true);
            team1_score++;
            Log(LogLevel::Info, "Team 1 has scored - Current score: Team 1 %d - %d Team 2",
                team1_score, team2_score);
            ball->SetVelocity({-20, -60});
        }
        if (object->GetName() == "basket_2") {
            // app->quit.store(true);
            team2_score++;
            Log(LogLevel::Info, "Team 1 has scored - Current score: Team 1 %d - %d Team 2",
                team1_score, team2_score);
            ball->SetVelocity({20, -60});
        }
    }
}

void UpdatePlayer(GameObject *player) {
    bool player_moved = false;

    if (app->key_map->key_right.pressed.load()) {
        player_moved = true;
        player->SetVelocity({20, player->GetVelocity().y}); // Move left
    } else if (app->key_map->key_left.pressed.load()) {
        player_moved = true;
        player->SetVelocity({-20, player->GetVelocity().y}); // Move right
    }

    if (app->key_map->key_up.pressed.load()) {
        player_moved = true;
        player->SetVelocity({player->GetVelocity().x, -20});
    }

    if (!player_moved) {
        player->SetVelocity({0, player->GetVelocity().y});
    }
}

void UpdateBall(GameObject *ball) {
    bool collision_with_player = false;
    bool collision_with_ground = false;
    bool collision_with_opponent = false;

    for (GameObject *collider : ball->GetColliders()) {
        std::string collider_name = collider->GetName();
        if (collider_name == "ground") {
            collision_with_ground = true;
        }
        if (collider_name.find("player") == 0) {
            collision_with_player = true;
            if (GetPlayerIdFromName(collider_name) < 3) {
                ball->SetVelocity({20, -60});
            } else {
                ball->SetVelocity({-20, -60});
            }
        }
        if (collider_name == "opponent") {
            collision_with_opponent = true;
            ball->SetVelocity({-20, -20});
        }
        if (collider_name == "basket") {
            // Once the ball hits the basket, it sticks to the basket
            ball->SetVelocity({0, 0});
        }
    }

    if (collision_with_ground) {
        ball->SetAcceleration({-ball->GetVelocity().x, 10});
    } else {
        ball->SetAcceleration({0, 10});
    }
}

GameObject *CreatePlayer(NetworkInfo network_info) {
    GameObject *player = new GameObject("player", Controllable);
    player->SetSize({50, 100});
    Log(LogLevel::Info, "Window width: %d, window height: %d", window_size.width,
        window_size.height);

    Log(LogLevel::Info, "Network id: %d", network_info.id);

    switch (network_info.id) {
    case 1:
        player->SetPosition({300, float(window_size.height - 300)});
        break;
    case 2:
        player->SetPosition({400, float(window_size.height - 300)});
        break;
    case 3:
        player->SetPosition({float(window_size.width - 300), float(window_size.height - 300)});
        break;
    case 4:
        player->SetPosition({float(window_size.width - 400), float(window_size.height - 300)});
        break;
    default:
        if (network_info.role != Server) {
            player->SetColor({0, 0, 0, 0});
            Log(LogLevel::Error, "More than 4 players spotted: EXITING THE GAME. Player ID: %d",
                network_info.id);
            app->quit.store(true);
        }
    }

    player->SetAcceleration({0, 10});
    player->SetTextureTemplate("assets/player_{}.png");
    player->SetOwner(NetworkRole::Client);
    player->SetCallback(UpdatePlayer);

    return player;
}

GameObject *CreateBall() {
    GameObject *ball = new GameObject("ball", Moving);
    ball->SetSize({50, 50});
    ball->SetPosition({float(window_size.width) / 2, float(window_size.height - 250)});
    ball->SetAcceleration({0, 10});
    ball->SetTexture("assets/soccer_ball.png");
    ball->SetOwner(NetworkRole::Server);
    ball->SetCallback(UpdateBall);

    return ball;
}

std::vector<GameObject *> CreateBaskets() {
    std::vector<GameObject *> baskets;

    GameObject *basket_1 = new GameObject("basket_1", Stationary);
    basket_1->SetSize({50, 75});
    basket_1->SetPosition({float(window_size.width - 100), float(window_size.height - 275)});
    basket_1->SetAcceleration({0, 10});
    basket_1->SetTexture("assets/basket_1.png");
    basket_1->SetOwner(NetworkRole::Server);
    basket_1->SetAffectedByCollision(false);
    baskets.push_back(basket_1);

    GameObject *basket_2 = new GameObject("basket_2", Stationary);
    basket_2->SetSize({50, 75});
    basket_2->SetPosition({float(100), float(window_size.height - 275)});
    basket_2->SetAcceleration({0, 10});
    basket_2->SetTexture("assets/basket_2.png");
    basket_2->SetOwner(NetworkRole::Server);
    basket_2->SetAffectedByCollision(false);
    baskets.push_back(basket_2);

    return baskets;
}

GameObject *CreateGround() {
    GameObject *ground = new GameObject("ground", Stationary);
    ground->SetSize({window_size.width, 200});
    ground->SetPosition({0, float(window_size.height - 200)});
    ground->SetAcceleration({0, 10});
    ground->SetColor({0, 255, 0, 255});
    ground->SetOwner(NetworkRole::Server);
    ground->SetAffectedByCollision(false);

    return ground;
}

// std::vector<GameObject *> CreateWalls() { return std::vector<GameObject *>(); }

std::vector<GameObject *> CreateGameObjects(NetworkInfo network_info) {
    GameObject *player = CreatePlayer(network_info);
    GameObject *ball = CreateBall();
    std::vector<GameObject *> baskets = CreateBaskets();
    GameObject *ground = CreateGround();

    std::vector<GameObject *> game_objects =
        std::vector({player, ball, baskets[0], baskets[1], ground});

    for (GameObject *game_object : game_objects) {
        if (network_info.mode == NetworkMode::PeerToPeer) {
            if (game_object->GetOwner() == NetworkRole::Server) {
                game_object->SetOwner(NetworkRole::Host);
            }
            if (game_object->GetOwner() == NetworkRole::Client) {
                game_object->SetOwner(NetworkRole::Peer);
            }
        }
    }

    return game_objects;
}

void DestroyObjects(std::vector<GameObject *> game_objects) {
    for (GameObject *object : game_objects) {
        delete object;
    }
}

int main(int argc, char *args[]) {
    std::string game_title = "Rohan's CSC581 HW2 Game: Multiplayer Backyard Soccer";
    int max_player_count = 4;

    // Initializing the Game Engine
    GameEngine game_engine;
    if (!SetEngineCLIOptions(&game_engine, argc, args)) {
        return 1;
    }

    if (!game_engine.Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    game_engine.SetPlayerTextures(max_player_count);
    game_engine.SetMaxPlayers(max_player_count);
    game_engine.SetShowPlayerBorder(true);

    NetworkInfo network_info = game_engine.GetNetworkInfo();
    if (network_info.id > 4) {
        Log(LogLevel::Error, "More than 4 players spotted: EXITING THE GAME. Player ID: %d",
            network_info.id);
        exit(0);
    }

    Color background_color = Color{165, 200, 255, 255};
    game_engine.SetBackgroundColor(background_color);
    game_engine.SetGameTitle(game_title);

    window_size = GetWindowSize();

    std::vector<GameObject *> game_objects = CreateGameObjects(network_info);
    game_engine.AddObjects(game_objects);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");

    // Add Game Cleanup code (deallocating pointers)
    DestroyObjects(game_objects);
    return 0;
}