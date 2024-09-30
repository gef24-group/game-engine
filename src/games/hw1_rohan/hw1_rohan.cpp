#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <string>
#include <vector>

// Head soccer
Size window_size;

void Update(std::vector<GameObject *> *game_objects) {
    GameObject *ball = NULL, *player = NULL;
    for (GameObject *object : *game_objects) {
        if (object->GetName() == "ball") {
            ball = object;
        }
        if (object->GetName() == "player") {
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

    // If the ball collides with the basket, the player wins: game ends
    for (GameObject *object : ball->GetColliders()) {
        if (object->GetName() == "basket") {
            app->quit.store(true);
            Log(LogLevel::Info, "You've scored a GOAL!! You win!");
        }
    }
}

void UpdatePlayer(GameObject *player) {
    bool player_moved = false;

    if (app->key_map->key_right.pressed.load()) {
        player_moved = true;
        player->SetVelocity({60, player->GetVelocity().y}); // Move left
    } else if (app->key_map->key_left.pressed.load()) {
        player_moved = true;
        player->SetVelocity({-60, player->GetVelocity().y}); // Move right
    }

    if (app->key_map->key_up.pressed.load()) {
        player_moved = true;
        player->SetVelocity({player->GetVelocity().x, -60});
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
        if (collider->GetName() == "ground") {
            collision_with_ground = true;
        }
        if (collider->GetName() == "player") {
            collision_with_player = true;
            // Log(LogLevel::Info, "The ball is being set to velocity: %d",
            // collider->GetVelocity().x);
            ball->SetVelocity(collider->GetVelocity());
            if (app->key_map->key_space.pressed.load()) {
                ball->SetVelocity({60, -60});
            }
        }
        if (collider->GetName() == "opponent") {
            collision_with_opponent = true;
            ball->SetVelocity({-60, -60});
        }
        if (collider->GetName() == "basket") {
            // Once the ball hits the basket, it sticks to the basket
            ball->SetVelocity({0, 0});
        }
    }

    if (collision_with_ground && !collision_with_player && !collision_with_opponent) {
        ball->SetAcceleration({-ball->GetVelocity().x / 2, 10});
    } else {
        ball->SetAcceleration({0, 10});
    }
}

void UpdateOpponent(GameObject *opponent) {
    Position opponent_position = opponent->GetPosition();
    if (opponent_position.x < float(window_size.width) - 400) {
        opponent->SetVelocity({30, 0});
    }
    if (opponent_position.x > float(window_size.width) - 200) {
        opponent->SetVelocity({-30, 0});
    }
}

GameObject *CreatePlayer() {
    GameObject *player = new GameObject("player", Controllable);
    player->SetSize({50, 100});
    player->SetPosition({300, float(window_size.height - 300)});
    player->SetAcceleration({0, 10});
    player->SetTexture("assets/soccer_player_right_facing.png");

    player->SetCallback(UpdatePlayer);

    return player;
}

GameObject *CreateOpponent() {
    GameObject *opponent = new GameObject("opponent", Moving);
    opponent->SetSize({50, 100});
    opponent->SetPosition({float(window_size.width - 300), float(window_size.height - 300)});
    opponent->SetVelocity({-30, 0});
    opponent->SetAcceleration({0, 10});
    opponent->SetTexture("assets/soccer_player_left_facing.png");
    opponent->SetRestitution(1);
    opponent->SetCallback(UpdateOpponent);

    return opponent;
}

GameObject *CreateBall() {
    GameObject *ball = new GameObject("ball", Moving);
    ball->SetSize({50, 50});
    ball->SetPosition({float(window_size.width) / 2, float(window_size.height - 250)});
    ball->SetAcceleration({0, 10});
    ball->SetTexture("assets/soccer_ball.png");
    ball->SetCallback(UpdateBall);

    return ball;
}

GameObject *CreateBasket() {
    GameObject *basket = new GameObject("basket", Stationary);
    basket->SetSize({50, 75});
    basket->SetPosition({float(window_size.width - 100), float(window_size.height - 275)});
    basket->SetAcceleration({0, 10});
    basket->SetTexture("assets/basket.png");
    basket->SetAffectedByCollision(false);

    return basket;
}

GameObject *CreateGround() {
    GameObject *ground = new GameObject("ground", Stationary);
    ground->SetSize({window_size.width, 200});
    ground->SetPosition({0, float(window_size.height - 200)});
    ground->SetAcceleration({0, 10});
    ground->SetColor({0, 255, 0, 255});
    ground->SetAffectedByCollision(false);

    return ground;
}

// std::vector<GameObject *> CreateWalls() { return std::vector<GameObject *>(); }

std::vector<GameObject *> CreateGameObjects() {
    GameObject *player = CreatePlayer();
    GameObject *opponent = CreateOpponent();
    GameObject *ball = CreateBall();
    GameObject *basket = CreateBasket();
    GameObject *ground = CreateGround();

    std::vector<GameObject *> game_objects = std::vector({player, opponent, ball, basket, ground});

    return game_objects;
}

void DestroyObjects(std::vector<GameObject *> game_objects) {
    for (GameObject *object : game_objects) {
        delete object;
    }
}

int main(int argc, char *args[]) {
    std::string game_title = "Rohan's CSC581 HW1 Game: Backyard Soccer";

    // Initializing the Game Engine
    GameEngine game_engine;
    if (!SetEngineCLIOptions(&game_engine, argc, args)) {
        return 1;
    }

    Color background_color = Color{165, 200, 255, 255};
    game_engine.SetBackgroundColor(background_color);
    game_engine.SetGameTitle(game_title);

    if (!game_engine.Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    window_size = GetWindowSize();

    std::vector<GameObject *> game_objects = CreateGameObjects();
    game_engine.SetGameObjects(game_objects);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");

    // Add Game Cleanup code (deallocating pointers)
    DestroyObjects(game_objects);
    return 0;
}