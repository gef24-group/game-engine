#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <string>

// Game update code
void Update(std::vector<GameObject *> game_objects) {
    // GameObject *ball = GetObjectByName("ball", game_objects);
    // Log(LogLevel::Info, "[ball] x: %f, y: %f, vX: %f, vY: %f, aX: %f, aY: %f",
    //     ball->GetPosition().x, ball->GetPosition().y, ball->GetVelocity().x,
    //     ball->GetVelocity().y, ball->GetAcceleration().x, ball->GetAcceleration().y);
    // Log(LogLevel::Info, "[ball colliders] %d", ball->GetColliders().size());
}

void UpdateEnemy(GameObject *enemy) {
    int right_edge = static_cast<int>(std::round(enemy->GetPosition().x)) + enemy->GetSize().width;
    int left_edge = static_cast<int>(std::round(enemy->GetPosition().x));

    // Check if the enemy hits the right edge of the window
    if (right_edge >= GetWindowSize().width && enemy->GetVelocity().x > 0) {
        // Reverse the direction by flipping the velocity
        enemy->SetVelocity({-50, enemy->GetVelocity().y});
    }

    // Check if the enemy hits the left edge of the window
    if (left_edge <= 0 && enemy->GetVelocity().x < 0) {
        // Reverse the direction by flipping the velocity
        enemy->SetVelocity({50, enemy->GetVelocity().y});
    }
}

void UpdateBall(GameObject *ball) {
    if (app->key_map->key_up) {
        if (ball->GetColliders().size() > 0) {
            ball->SetVelocity({ball->GetVelocity().x, -80});
        }
    }

    if (app->key_map->key_down) {
        ball->SetVelocity({ball->GetVelocity().x, 50});
    }

    if (app->key_map->key_left) {
        ball->SetVelocity({-100, ball->GetVelocity().y});
    } else if (app->key_map->key_right) {
        ball->SetVelocity({100, ball->GetVelocity().y});
    } else {
        ball->SetVelocity({0, ball->GetVelocity().y});
    }
}

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW1 Joshua's Game";

    // Initializing the Game Engine
    GameEngine game_engine;
    Color background_color = Color{143, 217, 251, 255};
    game_engine.SetBackgroundColor(background_color);
    if (!game_engine.Init(game_title.c_str())) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    GameObject ball("ball", Controllable);
    GameObject enemy("enemy", Moving);
    GameObject platform("platform", Stationary);

    ball.SetColor(Color{0, 0, 0, 0});
    ball.SetPosition(Position{0, 0});
    ball.SetSize(Size{100, 100});
    ball.SetAcceleration(Acceleration{0, 9.8});
    ball.SetVelocity(Velocity{0, 0});
    ball.SetRestitution(0.5);
    ball.SetTexture("assets/ball.png");
    ball.SetCallback(UpdateBall);

    enemy.SetColor(Color{255, 0, 0, 255});
    enemy.SetPosition(Position{20, 200});
    enemy.SetSize(Size{200, 50});
    enemy.SetVelocity(Velocity{50, 0});
    enemy.SetCallback(UpdateEnemy);

    platform.SetColor(Color{0, 0, 255, 255});
    platform.SetPosition(Position{0, 500});
    platform.SetSize(Size{900, 500});

    std::vector<GameObject *> objects = std::vector({&ball, &enemy, &platform});
    game_engine.AddObjects(objects);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    return 0;
}