#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "SDL_log.h"
#include "Types.hpp"
#include <iostream>
#include <string>

// Game update code
void Update(std::vector<GameObject *> game_objects) {
    for (GameObject *game_object : game_objects) {
        (*game_object).Update();
    }
}

void UpdateEnemy(GameObject *game_object) {
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "%f",
                   game_object->GetPosition().x);
}

void UpdateBall(GameObject *game_object) {
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO,
                   "x: %f, y: %f, vX: %f, vY: %f, aX: %f, aY: %f", game_object->GetPosition().x,
                   game_object->GetPosition().y, game_object->GetVelocity().x,
                   game_object->GetVelocity().y, game_object->GetAcceleration().x,
                   game_object->GetAcceleration().y);
}

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW1 Joshua's Game";

    // Initializing the Game Engine
    GameEngine game_engine;
    Color background_color = Color{143, 217, 251, 255};
    game_engine.SetBackgroundColor(background_color);
    if (!game_engine.Init(game_title.c_str())) {
        std::cout << "Game engine initialization failure";
        return 1;
    } else {
        GameObject ball(Controllable);
        GameObject enemy(Moving);
        GameObject platform(Stationary);

        ball.SetColor(Color{0, 0, 0, 0});
        ball.SetPosition(Position{500, 0});
        ball.SetSize(Size{100, 100});
        ball.SetAcceleration(Acceleration{0, 9.8});
        ball.SetVelocity(Velocity{0, 0});
        ball.SetTexture("assets/ball.png");
        ball.SetCallback(UpdateBall);

        enemy.SetColor(Color{0, 255, 0, 255});
        enemy.SetPosition(Position{20, 5});
        enemy.SetSize(Size{5, 5});
        // enemy.SetCallback(UpdateEnemy);

        platform.SetColor(Color{0, 0, 255, 255});
        platform.SetPosition(Position{10, 10});
        platform.SetSize(Size{80, 10});

        std::vector<GameObject *> objects = std::vector({&ball, &enemy, &platform});
        game_engine.AddObjects(objects);
        game_engine.SetCallback(Update);

        // The Start function keeps running until an "exit event occurs"
        game_engine.Start();
        // Code for graceful shutdown
        game_engine.Shutdown();
    }
    std::cout << "The game engine has closed the game cleanly" << std::endl;
}