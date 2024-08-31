#include "GameEngine.hpp"
#include "GameObject.hpp"
#include <iostream>
#include <string>

// Game update code
void Update(std::vector<GameObject *> game_objects) {
    for (GameObject *game_object : game_objects) {
        (*game_object).Update();
    }
}

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW1 Mitesh's Game";

    // Initializing the Game Engine
    GameEngine game_engine;
    Color background_color = Color{143, 217, 251, 255};
    game_engine.SetBackgroundColor(background_color);
    if (!game_engine.Init(game_title.c_str())) {
        std::cout << "Game engine initialization failure";
        return 1;
    } else {
        GameObject paddle(Controllable);
        GameObject snake(Moving);
        GameObject wall(Stationary);

        std::vector<GameObject *> objects = std::vector({&paddle, &snake, &wall});
        game_engine.AddObjects(objects);
        game_engine.SetCallback(Update);

        // The Start function keeps running until an "exit event occurs"
        game_engine.Start();
        // Code for graceful shutdown
        game_engine.Shutdown();
    }
    std::cout << "The game engine has closed the game cleanly" << std::endl;
}