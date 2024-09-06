#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <string>

Size window_size;

// Game update code
void Update(std::vector<GameObject *> *game_objects) {

    // Get the ball and platform objects
    GameObject *ball = GetObjectByName("ball", *game_objects);
    GameObject *platform = GetObjectByName("platform", *game_objects);

    // Update ball's position and handle collisions
    Position ball_pos = ball->GetPosition();
    Velocity ball_vel = ball->GetVelocity();
    Size ball_size = ball->GetSize();

    // Wall boundaries
    GameObject *wall_left = GetObjectByName("wall_left", *game_objects);
    GameObject *wall_right = GetObjectByName("wall_right", *game_objects);
    GameObject *wall_top = GetObjectByName("wall_top", *game_objects);

    wall_left->SetPosition(Position{0, 0});
    wall_left->SetSize(Size{100, window_size.height});

    wall_top->SetPosition(Position{100, 0});
    wall_top->SetSize(Size{window_size.width - 100, 100});

    wall_right->SetPosition(Position{float(window_size.width - 100), 100});
    wall_right->SetSize(Size{100, window_size.height - 100});

    // Collision with left wall
    if (ball_pos.x <= wall_left->GetPosition().x + wall_left->GetSize().width) {
        ball->SetVelocity({-ball_vel.x, ball_vel.y}); // Reflect horizontally
    }

    // Collision with right wall
    if (ball_pos.x + ball_size.width >= wall_right->GetPosition().x) {
        ball->SetVelocity({-ball_vel.x, ball_vel.y}); // Reflect horizontally
    }

    // Collision with top wall
    if (ball_pos.y <= wall_top->GetPosition().y + wall_top->GetSize().height) {
        ball->SetVelocity({ball_vel.x, -ball_vel.y}); // Reflect vertically
    }

    // Collision with platform
    if (ball_pos.y + ball_size.height >= platform->GetPosition().y &&
        ball_pos.x + ball_size.width >= platform->GetPosition().x &&
        ball_pos.x <= platform->GetPosition().x + platform->GetSize().width) {
        ball->SetVelocity({ball_vel.x, -ball_vel.y}); // Reflect vertically
    }

    // If ball goes out of the boundary
    if (ball_pos.y + ball_size.height > 1080) {
        Log(LogLevel::Info, "You lost!");
        app->quit = true;
    }
}

void UpdatePlatform(GameObject *platform) {

    if (app->key_map->key_left) {
        platform->SetPosition({platform->GetPosition().x - 10, platform->GetPosition().y});
    } else if (app->key_map->key_right) {
        platform->SetPosition({platform->GetPosition().x + 10, platform->GetPosition().y});
    }
}

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW1 Mitesh's Game";

    // Initializing the Game Engine
    GameEngine game_engine;
    Color background_color = Color{0, 0, 0, 255};
    game_engine.SetBackgroundColor(background_color);
    if (!game_engine.Init(game_title.c_str())) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    window_size = GetWindowSize();

    GameObject ball("ball", Moving);
    GameObject wall_left("wall_left", Stationary);
    GameObject wall_top("wall_top", Stationary);
    GameObject wall_right("wall_right", Stationary);
    GameObject platform("platform", Controllable);

    ball.SetColor(Color{0, 0, 0, 0});
    ball.SetPosition(Position{200, 200});
    ball.SetSize(Size{50, 50});
    ball.SetAcceleration(Acceleration{0, 0});
    ball.SetVelocity(Velocity{40, 40});
    ball.SetRestitution(1);
    ball.SetTexture("assets/ball.png");
    ball.SetReduceVelocityOnCollision(false);

    wall_left.SetColor(Color{255, 0, 0, 255});
    wall_left.SetPosition(Position{0, 0});
    wall_left.SetSize(Size{100, 1080});
    wall_left.SetTexture("assets/wall.jpeg");

    wall_top.SetColor(Color{255, 0, 0, 255});
    wall_top.SetPosition(Position{100, 0});
    wall_top.SetSize(Size{1820, 100});
    wall_top.SetTexture("assets/wall.jpeg");

    wall_right.SetColor(Color{255, 0, 0, 255});
    wall_right.SetPosition(Position{1620, 100});
    wall_right.SetSize(Size{100, 980});
    wall_right.SetTexture("assets/wall.jpeg");
    // enemy.SetVelocity(Velocity{50, 0});
    // enemy.SetCallback(UpdateEnemy);

    platform.SetColor(Color{128, 128, 128, 255});
    platform.SetPosition(Position{250, 1010});
    platform.SetSize(Size{200, 30});
    platform.SetCallback(UpdatePlatform);

    std::vector<GameObject *> objects =
        std::vector({&ball, &wall_left, &wall_top, &wall_right, &platform});
    game_engine.AddObjects(objects);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    return 0;
}