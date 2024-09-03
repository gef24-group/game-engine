#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <string>

// Game update code
void Update(std::vector<GameObject *> game_objects) {
    for (GameObject *game_object : game_objects) {
        game_object->SetShape(Rectangle);
        game_object->SetSize(Size{50, 50});
        // game_object->Update();
        // objects response after collision
        Velocity curr_obj_velocity = game_object->GetVelocity();

        if (game_object->GetColliders().size() > 0 && (game_object->GetCategory() != Stationary)) {
            for (GameObject *collider : game_object->GetColliders()) {
                if (collider->GetAngle() == 0) {
                    game_object->SetVelocity({curr_obj_velocity.x, -curr_obj_velocity.y});
                } else if (collider->GetAngle() == 90) {
                    game_object->SetVelocity({-curr_obj_velocity.x, curr_obj_velocity.y});
                } else {
                    game_object->SetVelocity({0, 0});
                }
            }
        }
    }
}

void UpdatePaddle(GameObject *paddle) {
    if (app->key_map->key_right) {
        paddle->SetVelocity({3, 3}); // Move left
    }
    if (app->key_map->key_left) {
        paddle->SetVelocity({-3, -3}); // Move right
    }
}

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW1 Rohan's Game";

    // Initializing the Game Engine
    GameEngine game_engine;
    Color background_color = Color{143, 217, 251, 255};
    game_engine.SetBackgroundColor(background_color);
    if (!game_engine.Init(game_title.c_str())) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    GameObject paddle("paddle", Controllable);
    paddle.SetPosition(Position{10, 10});
    paddle.SetCallback(UpdatePaddle);
    GameObject ball("ball", Moving);
    ball.SetPosition(Position{210, 210});
    GameObject wall("wall", Stationary);
    wall.SetPosition(Position{410, 410});

    std::vector<GameObject *> objects = std::vector({&paddle, &ball, &wall});
    game_engine.AddObjects(objects);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    return 0;
}