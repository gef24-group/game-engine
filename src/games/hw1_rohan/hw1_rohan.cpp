#include "GameEngine.hpp"
#include <iostream>
#include <string>

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW1 Rohan's Game";

    // Initializing the Game Engine
    GameEngine game_engine;
    if (!game_engine.Init(game_title.c_str())) {
        std::cout << "Game engine initialization failure";
        return 1;
    } else {
        game_engine.Start();
    }
    std::cout << "The game engine has closed the game" << std::endl;
}