#pragma once
#include "structs.hpp"
#include <SDL2/SDL.h>

// SDL render and window context
extern App *app;

// Prepare scene to be rendered to window
void prepareScene(void);
// Render scene to window
void presentScene(void);