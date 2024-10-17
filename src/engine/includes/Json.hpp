#pragma once

#include "Types.hpp"
#include <nlohmann/json.hpp>

// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
void to_json(nlohmann::json &json, const Position &position);
// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
void from_json(const nlohmann::json &json, Position &position);

// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
void to_json(nlohmann::json &json, const EntityUpdate &entity_update);
// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
void from_json(const nlohmann::json &json, EntityUpdate &entity_update);