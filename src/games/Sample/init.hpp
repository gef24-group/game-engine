#pragma once
#include "defs.hpp"
#include "structs.hpp"
#include <SDL2/SDL.h>
#include <iostream>

// SDL render and window context
extern App *app;

// Initialize SDL rendering window
void initSDL(void);