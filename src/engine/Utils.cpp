#include "Utils.hpp"
#include "SDL_image.h"
#include "SDL_log.h"
#include "Types.hpp"

SDL_Texture *LoadTexture(std::string path) {
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (surface == NULL) {
        Log(LogLevel::Error, "Error: \'%s\' while loading the image file: %s", IMG_GetError(),
            path.c_str());
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    if (texture == NULL) {
        Log(LogLevel::Error,
            "Error: '%s' while creating texture from surface for the image file: %s",
            SDL_GetError(), path.c_str());
        return NULL;
    }

    return texture;
}

void Log(LogLevel log_level, const char *fmt, ...) {
    SDL_LogPriority log_priority;
    switch (log_level) {
    case LogLevel::Verbose:
        log_priority = SDL_LOG_PRIORITY_VERBOSE;
        break;
    case LogLevel::Debug:
        log_priority = SDL_LOG_PRIORITY_DEBUG;
        break;
    case LogLevel::Info:
        log_priority = SDL_LOG_PRIORITY_INFO;
        break;
    case LogLevel::Warn:
        log_priority = SDL_LOG_PRIORITY_WARN;
        break;
    case LogLevel::Error:
        log_priority = SDL_LOG_PRIORITY_ERROR;
        break;
    case LogLevel::Critical:
        log_priority = SDL_LOG_PRIORITY_CRITICAL;
        break;
    case LogLevel::Priorities:
        log_priority = SDL_NUM_LOG_PRIORITIES;
        break;
    default:
        log_priority = SDL_LOG_PRIORITY_INFO;
    }

    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, log_priority, fmt, args);
    va_end(args);
}