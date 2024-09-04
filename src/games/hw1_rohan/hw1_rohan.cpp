#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <string>
#include <vector>

// Head soccer
Size window_size;

// Game update code
void Update(std::vector<GameObject *> *game_objects) {
    // for (GameObject *game_object : *game_objects) {
    //     game_object->SetShape(Rectangle);
    //     game_object->SetSize(Size{50, 50});
    //     game_object->Update();
    //     // objects response after collision
    //     Velocity curr_obj_velocity = game_object->GetVelocity();

    //     if (game_object->GetColliders().size() > 0 && (game_object->GetCategory() != Stationary))
    //     {
    //         for (GameObject *collider : game_object->GetColliders()) {
    //             if (collider->GetAngle() == 0) {
    //                 game_object->SetVelocity({curr_obj_velocity.x, -curr_obj_velocity.y});
    //             } else if (collider->GetAngle() == 90) {
    //                 game_object->SetVelocity({-curr_obj_velocity.x, curr_obj_velocity.y});
    //             } else {
    //                 game_object->SetVelocity({0, 0});
    //             }
    //         }
    //     }
    // }
}

void UpdatePlayer(GameObject *player) {
    if (app->key_map->key_right) {
        player->SetVelocity({15, 0}); // Move left
    }
    if (app->key_map->key_left) {
        player->SetVelocity({-15, 0}); // Move right
    }
    if (app->key_map->key_up) {
        player->SetVelocity({0, -15});
    }

    if (app->key_map->key_space) {
        for (GameObject *collider : player->GetColliders()) {
            Log(LogLevel::Info, "Colliding with a %s", collider->GetName().c_str());
            if (collider->GetName() == "ball") {
                Log(LogLevel::Info, "Setting ball velocity");
                collider->SetVelocity({30, -30});
            }
        }
        Log(LogLevel::Info, "SPACE KEY: %d", app->key_map->key_space);
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
    opponent->SetAcceleration({0, 10});
    opponent->SetTexture("assets/soccer_player_left_facing.png");

    return opponent;
}

GameObject *CreateBall() {
    GameObject *ball = new GameObject("ball", Moving);
    ball->SetSize({50, 50});
    ball->SetPosition({float(window_size.width) / 2, float(window_size.height - 250)});
    ball->SetAcceleration({0, 10});
    ball->SetTexture("assets/soccer_ball.png");

    return ball;
}

GameObject *CreateBasket() {
    GameObject *basket = new GameObject("basket", Stationary);
    basket->SetSize({50, 75});
    basket->SetPosition({float(window_size.width - 100), float(window_size.height - 275)});
    basket->SetAcceleration({0, 10});
    basket->SetTexture("assets/basket.png");

    return basket;
}

GameObject *CreateGround() {
    GameObject *ground = new GameObject("ground", Stationary);
    ground->SetSize({window_size.width, 200});
    ground->SetPosition({0, float(window_size.height - 200)});
    ground->SetAcceleration({0, 10});
    ground->SetColor({0, 255, 0, 255});

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

int main(int argc, char *args[]) {
    std::string game_title = "Rohan's CSC581 HW1 Game: Basket Soccer";

    // Initializing the Game Engine
    GameEngine game_engine;
    Color background_color = Color{165, 200, 255, 255};
    game_engine.SetBackgroundColor(background_color);
    if (!game_engine.Init(game_title.c_str())) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    window_size = GetWindowSize();
    // std::vector<GameObject *> game_objects =
    //     CreateGameObjects(window_size.width, window_size.height);

    //     CreatePlayer();
    //     CreateOpponent();
    //     CreateBall();
    //     CreateBasket();

    //      UpdatePlayer();
    //      UpdateOpponent();

    // GameObject paddle("paddle", Controllable);
    // paddle.SetPosition(Position{130, 810});
    // paddle.SetCallback(UpdatePaddle);
    // GameObject ball("ball", Moving);
    // ball.SetPosition(Position{210, 210});
    // ball.SetVelocity(Velocity{0, 7});
    // // ball.SetAcceleration(Acceleration{0, 10});
    // GameObject wall("wall", Stationary);
    // wall.SetPosition(Position{410, 410});

    std::vector<GameObject *> game_objects = CreateGameObjects();
    game_engine.AddObjects(game_objects);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");

    // Add Game Cleanup code (deallocating pointers)
    return 0;
}