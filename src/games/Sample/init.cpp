#include "init.hpp"

void initSDL(void) {
    int rendererFlags, windowFlags;
    // Name the window
    std::string windowName = "CSC 481/581 Project";

    // Some Render flags
    rendererFlags = SDL_RENDERER_ACCELERATED;

    // Nothing of note to add to the window here
    windowFlags = 0;

    // If the SDL initialization is unsuccessful
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Couldn't initialize SDL 2 : " << SDL_GetError();
        exit(1);
    }

    // If SDL initialized successfully, attempt to create the SDL window.
    app->window =
        SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

    // If the window pointer is null we have an error
    if (!app->window) {
        std::cout << "Failed to open Window: " << SDL_GetError();
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    // Create the renderer for the window
    app->renderer = SDL_CreateRenderer(app->window, -1, rendererFlags);

    // If the renderer failed to initialize (returned pointer is null)
    if (!app->renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }
}