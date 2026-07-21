#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace ecs {

class EntityFactory {
public:
    // Create an entity in the registry using a JSON object containing its components
    static entt::entity CreateEntity(entt::registry& registry, const nlohmann::json& entityJson);

    // Load a prefab file (JSON array of entities) and spawn them in the registry
    static std::vector<entt::entity> LoadPrefab(entt::registry& registry, const std::string& filepath);
};

} // namespace ecs
