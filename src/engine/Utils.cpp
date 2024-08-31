#include "Utils.hpp"
#include "SDL_image.h"

SDL_Texture *LoadTexture(std::string path) {
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (surface == NULL) {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                       "Error: \'%s\' while loading the image file: %s", IMG_GetError(),
                       path.c_str());
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    if (texture == NULL) {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                       "Error: '%s' while creating texture from surface for the image file: %s",
                       SDL_GetError(), path.c_str());
        return NULL;
    }

    return texture;
}
