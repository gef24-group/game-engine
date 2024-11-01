#include "Utils.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Network.hpp"
#include "Render.hpp"
#include "SDL_image.h"
#include "SDL_log.h"
#include "Types.hpp"
#include <algorithm>
#include <random>
#include <vector>

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
    SDL_FreeSurface(surface);

    if (texture == NULL) {
        Log(LogLevel::Error,
            "Error: '%s' while creating texture from surface for the image file: %s",
            SDL_GetError(), path.c_str());
        return NULL;
    }

    if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) != 0) {
        Log(LogLevel::Error, "Error: '%s' while setting blend mode for texture: %s", SDL_GetError(),
            path.c_str());
        SDL_DestroyTexture(texture);
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

Size GetWindowSize() { return Size{app->window.width, app->window.height}; }

Overlap GetOverlap(SDL_Rect rect_1, SDL_Rect rect_2) {
    Overlap overlap;

    int left_overlap = (rect_1.x + rect_1.w) - rect_2.x;
    int right_overlap = (rect_2.x + rect_2.w) - rect_1.x;
    int top_overlap = (rect_1.y + rect_1.h) - rect_2.y;
    int bottom_overlap = (rect_2.y + rect_2.h) - rect_1.y;

    int min_overlap =
        std::min(std::min(left_overlap, right_overlap), std::min(top_overlap, bottom_overlap));

    if (min_overlap == left_overlap) {
        overlap = Overlap::Left;
    } else if (min_overlap == right_overlap) {
        overlap = Overlap::Right;

    } else if (min_overlap == top_overlap) {
        overlap = Overlap::Top;

    } else if (min_overlap == bottom_overlap) {
        overlap = Overlap::Bottom;
    }
    return overlap;
}

Entity *GetEntityByName(std::string name, std::vector<Entity *> entities) {
    for (Entity *entity : entities) {
        if (entity->GetName() == name) {
            return entity;
        }
    }
    return nullptr;
}

Entity *GetControllable(std::vector<Entity *> entities) {
    for (Entity *entity : entities) {
        if (entity->GetCategory() == EntityCategory::Controllable) {
            return entity;
        }
    }
    return nullptr;
}

int GetControllableCount(std::vector<Entity *> entities) {
    int controllable_count = 0;

    for (Entity *entity : entities) {
        if (entity->GetCategory() == EntityCategory::Controllable) {
            controllable_count += 1;
        }
    }

    return controllable_count;
}

std::vector<Entity *> GetEntitiesByRole(NetworkInfo network_info, std::vector<Entity *> entities) {
    std::vector<Entity *> entity_list;

    if (network_info.mode == NetworkMode::Single) {
        return entities;
    }

    if (network_info.role == NetworkRole::Server || network_info.role == NetworkRole::Host) {
        for (auto *entity : entities) {
            if (entity->GetComponent<Network>() != nullptr &&
                entity->GetComponent<Network>()->GetOwner() == network_info.role) {
                entity_list.push_back(entity);
            }
        }
    }

    if (network_info.role == NetworkRole::Client || network_info.role == NetworkRole::Peer) {
        entity_list.push_back(GetClientPlayer(network_info.id, entities));
    }

    return entity_list;
}

void SetPlayerTexture(Entity *controllable, int player_id, int player_textures) {
    std::string texture_template = controllable->GetComponent<Render>()->GetTextureTemplate();
    size_t pos = texture_template.find("{}");
    player_id = (player_id - 1) % player_textures + 1;

    if (pos != std::string::npos) {
        texture_template.replace(pos, 2, std::to_string(player_id));
    }

    controllable->GetComponent<Render>()->SetTexture(texture_template);
}

Entity *GetClientPlayer(int player_id, std::vector<Entity *> entities) {
    for (Entity *entity : entities) {
        if (entity->GetCategory() == EntityCategory::Controllable) {
            std::string name = entity->GetName();
            size_t underscore = name.rfind('_');

            if (underscore != std::string::npos && (underscore + 1) < name.size()) {
                std::string number = name.substr(underscore + 1);
                try {
                    int player = std::stoi(number);
                    if (player == player_id) {
                        return entity;
                    }
                } catch (const std::invalid_argument &e) {
                    Log(LogLevel::Error, "Could not locate the client player to update");
                }
            }
        }
    }
    return nullptr;
}

bool SetEngineCLIOptions(int argc, char *args[]) {
    std::string mode;
    std::string role;
    std::string server_ip;
    std::string host_ip;
    std::string peer_ip;
    std::string encoding;
    std::vector<std::string> valid_modes = {"single", "cs", "p2p"};
    std::vector<std::string> valid_roles = {"server", "client", "host", "peer"};
    std::vector<std::string> valid_encodings = {"struct", "json"};

    for (int i = 1; i < argc; i++) {
        std::string arg = args[i];

        if (arg == "--mode" && i + 1 < argc) {
            mode = args[i + 1];
            i++;
        } else if (arg == "--role" && i + 1 < argc) {
            role = args[i + 1];
            i++;
        } else if (arg == "--server_ip" && i + 1 < argc) {
            server_ip = args[i + 1];
            i++;
        } else if (arg == "--host_ip" && i + 1 < argc) {
            host_ip = args[i + 1];
            i++;
        } else if (arg == "--peer_ip" && i + 1 < argc) {
            peer_ip = args[i + 1];
            i++;
        } else if (arg == "--encoding" && i + 1 < argc) {
            encoding = args[i + 1];
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
        Log(LogLevel::Error, "Invalid role. Must be one of [server, client, host, peer]");
        return false;
    }

    if ((mode == "single" && (role == "server" || role == "host" || role == "peer")) ||
        (mode == "cs" && (role == "host" || role == "peer")) ||
        (mode == "p2p" && (role == "server" || role == "client"))) {
        Log(LogLevel::Error, "[%s] mode does not support [%s] role!", mode.c_str(), role.c_str());
        return false;
    }

    if (!server_ip.empty() && !(mode == "cs" && role == "client")) {
        Log(LogLevel::Error,
            "--server_ip is only supported in the [cs] mode and the [client] role");
        return false;
    }
    if (!host_ip.empty() && !(mode == "p2p" && role == "peer")) {
        Log(LogLevel::Error, "--host_ip is only supported in the [p2p] mode and the [peer] role");
        return false;
    }
    if (!peer_ip.empty() && !(mode == "p2p" && role == "peer")) {
        Log(LogLevel::Error, "--peer_ip is only supported in the [p2p] mode and the [peer] role");
        return false;
    }
    if ((mode == "p2p" && role == "peer") && (!host_ip.empty() && peer_ip.empty())) {
        Log(LogLevel::Error, "Please specify --peer_ip!");
        return false;
    }
    if ((mode == "p2p" && role == "peer") && (host_ip.empty() && !peer_ip.empty())) {
        Log(LogLevel::Error, "Please specify --host_ip!");
        return false;
    }

    if (mode == "cs" && role == "client" && server_ip.empty()) {
        server_ip = "localhost";
    }
    if (mode == "p2p" && role == "peer") {
        if (host_ip.empty()) {
            host_ip = "localhost";
        }
        if (peer_ip.empty()) {
            peer_ip = "localhost";
        }
    }

    if (!encoding.empty() && mode == "single") {
        Log(LogLevel::Error, "--encoding is not supported in the [single] mode!");
        return false;
    }
    if (encoding.empty()) {
        encoding = "struct";
    }
    if (std::find(valid_encodings.begin(), valid_encodings.end(), encoding) ==
        valid_encodings.end()) {
        Log(LogLevel::Error, "Invalid encoding. Must be one of [struct, json]");
        return false;
    }

    NetworkMode network_mode;
    NetworkRole network_role;
    Encoding engine_encoding;

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
    if (role == "host") {
        network_role = NetworkRole::Host;
    }
    if (role == "peer") {
        network_role = NetworkRole::Peer;
    }

    if (encoding == "struct") {
        engine_encoding = Encoding::Struct;
    }
    if (encoding == "json") {
        engine_encoding = Encoding::JSON;
    }

    Engine::GetInstance().SetNetworkInfo(
        NetworkInfo{network_mode, network_role, 0, server_ip, host_ip, peer_ip});
    Engine::GetInstance().SetEncoding(engine_encoding);

    return true;
}

std::string GetConnectionAddress(std::string address, int port) {
    return "tcp://" + address + ":" + std::to_string(port);
}

// Signal handler for SIGINT
void HandleSIGINT(int signum) {
    app->sigint.store(true);
    Log(LogLevel::Info, "SIGINT received!");
}

Position GetScreenPosition(Position world_position, Position camera_position) {
    float screen_pos_x, screen_pos_y;

    screen_pos_x = world_position.x - camera_position.x;
    screen_pos_y = world_position.y - camera_position.y;

    return Position{screen_pos_x, screen_pos_y};
}

int GetPlayerIdFromName(std::string player_name) {
    std::vector<std::string> player = Split(player_name, '_');

    if (player.size() == 2) {
        return std::stoi(player[1]);
    }

    return -1;
}

std::vector<std::string> Split(std::string str, char delimiter) {
    std::vector<std::string> result;
    std::string current;

    for (char chr : str) {
        if (chr == delimiter) {
            // When delimiter is encountered, push the current substring to the result
            if (!current.empty()) {
                result.push_back(current);
                current.clear(); // Reset the current substring
            }
        } else {
            // If the character is not a delimiter, append it to the current substring
            current += chr;
        }
    }

    // Add the last part of the string
    if (!current.empty()) {
        result.push_back(current);
    }

    return result;
}

std::vector<Entity *> GetEntitiesByCategory(std::vector<Entity *> entities,
                                            EntityCategory category) {
    std::vector<Entity *> filtered_entities;
    for (Entity *entity : entities) {
        if (entity->GetCategory() == category) {
            filtered_entities.push_back(entity);
        }
    }

    return filtered_entities;
}

// Returns an integer between and including 0 and n
int GetRandomInt(int n) {
    std::random_device rand_dev;
    std::mt19937 gen(rand_dev());
    std::uniform_int_distribution<> distr(0, n);

    return distr(gen);
}
