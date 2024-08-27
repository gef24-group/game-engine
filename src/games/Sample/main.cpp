#include "main.hpp"
#include <memory>

int main(int argc, char *argv[]) {
    // Call the initialization functions
    initSDL();

    while (1) {
        // Prep the scene
        prepareScene();

        // Process input
        doInput();

        // modify the game world here

        // Present the resulting scene
        presentScene();

        // Inserting 16ms delay at the end for a budget frame-limiter.
        SDL_Delay(16);
    }

    return 0;
}