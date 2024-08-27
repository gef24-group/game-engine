#include "GameEngine.hpp"

int main(int argc, char *args[]) {
    std::string gameTitle = "CSC581 HW1 Rohan's Game";

    // Initializing the Game Engine
    GameEngine ge;
    if (!ge.init(gameTitle.c_str())) {
        std::cout << "Game engine initialization failure";
        return 1;
    } else {
        ge.start();
    }
    std::cout << "The game engine has closed the game" << std::endl;
}