#include "Json.hpp"

void to_json(nlohmann::json &json, const Position &position) {
    json = nlohmann::json{{"x", position.x}, {"y", position.y}};
}

void from_json(const nlohmann::json &json, Position &position) {
    json.at("x").get_to(position.x);
    json.at("y").get_to(position.y);
}

void to_json(nlohmann::json &json, const EntityUpdate &entity_update) {
    json = nlohmann::json{{"name", entity_update.name},
                          {"position", entity_update.position},
                          {"player_address", entity_update.player_address},
                          {"active", entity_update.active}};
}

void from_json(const nlohmann::json &json, EntityUpdate &entity_update) {
    std::string name = json.at("name").get<std::string>();
    std::strncpy(entity_update.name, name.c_str(), sizeof(entity_update.name));

    std::string player_address = json.at("player_address").get<std::string>();
    std::strncpy(entity_update.player_address, player_address.c_str(),
                 sizeof(entity_update.player_address));

    json.at("position").get_to(entity_update.position);
    json.at("active").get_to(entity_update.active);
}