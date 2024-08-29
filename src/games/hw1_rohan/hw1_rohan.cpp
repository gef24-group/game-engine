#include "GameEngine.hpp"
#include "GameObject.hpp"
#include <iostream>
#include <string>

// Game update code
void Update(std::vector<GameObject> game_objects) {
    for (GameObject game_object : game_objects) {
        game_object.Update();
    }
    // Add collision checker
}

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW1 Rohan's Game";

    // Initializing the Game Engine
    GameEngine game_engine;
    if (!game_engine.Init(game_title.c_str())) {
        std::cout << "Game engine initialization failure";
        return 1;
    } else {
        GameObject paddle(Controllable);
        GameObject ball(Moving);
        GameObject wall(Stationary);

        game_engine.AddObject(paddle);
        game_engine.AddObject(ball);
        game_engine.AddObject(wall);

        game_engine.SetCallback(Update);

        game_engine.Start();
    }
    std::cout << "The game engine has closed the game cleanly" << std::endl;
}