#include "Utils.hpp"
#include "GameEngine.hpp"
#include "SDL_image.h"
#include "SDL_log.h"
#include "SDL_video.h"
#include "Types.hpp"
#include <algorithm>

SDL_Texture *LoadTexture(std::string path) {
    if (app->renderer == nullptr) {
        return NULL;
    }

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

Size GetWindowSize() {
    int width = 0, height = 0;
    SDL_GetWindowSize(app->window, &width, &height);
    return Size{width, height};
}

GameObject *GetObjectByName(std::string name, std::vector<GameObject *> game_objects) {
    for (GameObject *game_object : game_objects) {
        if (game_object->GetName() == name) {
            return game_object;
        }
    }
    return nullptr;
}

GameObject *GetControllable(std::vector<GameObject *> game_objects) {
    for (GameObject *game_object : game_objects) {
        if (game_object->GetCategory() == GameObjectCategory::Controllable) {
            return game_object;
        }
    }
    return nullptr;
}

std::vector<GameObject *> GetObjectsByRole(NetworkInfo network_info,
                                           std::vector<GameObject *> game_objects) {
    std::vector<GameObject *> objects;

    if (network_info.mode == NetworkMode::Single) {
        return game_objects;
    }

    if (network_info.mode == NetworkMode::ClientServer ||
        network_info.mode == NetworkMode::PeerToPeer) {
        for (auto *game_object : game_objects) {
            if (game_object->GetOwner() == network_info.role) {
                objects.push_back(game_object);
            }
        }
    }

    return objects;
}

void SetPlayerTexture(GameObject *controllable, int player_id) {
    std::string texture_template = controllable->GetTextureTemplate();
    size_t pos = texture_template.find("{}");

    if (pos != std::string::npos) {
        texture_template.replace(pos, 2, std::to_string(player_id));
    }

    controllable->SetTexture(texture_template);
}

GameObject *GetClientPlayer(int player_id, std::vector<GameObject *> game_objects) {
    for (GameObject *game_object : game_objects) {
        if (game_object->GetCategory() == GameObjectCategory::Controllable) {
            std::string name = game_object->GetName();
            size_t underscore = name.rfind('_');

            if (underscore != std::string::npos && (underscore + 1) < name.size()) {
                std::string number = name.substr(underscore + 1);
                try {
                    int player = std::stoi(number);
                    if (player == player_id) {
                        return game_object;
                    }
                } catch (const std::invalid_argument &e) {
                    Log(LogLevel::Error, "Could not locate the client player to update");
                }
            }
        }
    }
    return nullptr;
}

bool SetEngineCLIOptions(GameEngine *game_engine, int argc, char *args[]) {
    std::string mode;
    std::string role;
    std::vector<std::string> valid_modes = {"single", "cs", "p2p"};
    std::vector<std::string> valid_roles = {"server", "client", "peer"};

    for (int i = 1; i < argc; i++) {
        std::string arg = args[i];

        if (arg == "--mode" && i + 1 < argc) {
            mode = args[i + 1];
            i++;
        } else if (arg == "--role" && i + 1 < argc) {
            role = args[i + 1];
            i++;
        }
    }
    if (mode.empty()) {
        mode = "single";
    }
    if (role.empty()) {
        role = "client";
    }

    if (std::find(valid_modes.begin(), valid_modes.end(), mode) == valid_modes.end()) {
        Log(LogLevel::Error, "Invalid mode. Must be one of [single, cs, p2p]");
        return false;
    }

    if (std::find(valid_roles.begin(), valid_roles.end(), role) == valid_roles.end()) {
        Log(LogLevel::Error, "Invalid role. Must be one of [server, client, peer]");
        return false;
    }

    if ((mode == "single" && (role == "server" || role == "peer")) ||
        (mode == "cs" && role == "peer") || (mode == "p2p" && role == "client")) {
        Log(LogLevel::Error, "[%s] mode does not support [%s] role!", mode.c_str(), role.c_str());
    }

    NetworkMode network_mode;
    NetworkRole network_role;

    if (mode == "single") {
        network_mode = NetworkMode::Single;
    }
    if (mode == "cs") {
        network_mode = NetworkMode::ClientServer;
    }
    if (mode == "p2p") {
        network_mode = NetworkMode::PeerToPeer;
    }

    if (role == "server") {
        network_role = NetworkRole::Server;
    }
    if (role == "client") {
        network_role = NetworkRole::Client;
    }
    if (role == "peer") {
        network_role = NetworkRole::Peer;
    }

    game_engine->SetNetworkInfo(NetworkInfo{network_mode, network_role, 0});

    return true;
}

// Signal handler for SIGINT
void HandleSIGINT(int signum) {
    app->sigint.store(true);
    Log(LogLevel::Info, "SIGINT received!");
}

int GetPlayerIdFromName(std::string player_name) {
    unsigned long player_name_length = player_name.size();
    unsigned long index = player_name_length - 1;
    // Traverse backwards to find the start of the integer
    while (index >= 0 && std::isdigit(player_name[index])) {
        index--;
    }

    if (index < player_name_length - 1) {
        return std::stoi(player_name.substr(index + 1));
    }

    // If no integers were found at the end of the player name
    return -1;
}
