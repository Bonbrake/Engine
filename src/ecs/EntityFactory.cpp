#include "ze/ecs/EntityFactory.h"
#include "ze/ecs/Components.h"
#include "ze/debug/MetaRegistry.h"
#include "ze/core/PRNG.h"
#include "ze/core/Logger.h"
#include <fstream>

namespace ecs {

entt::entity EntityFactory::CreateEntity(entt::registry& registry, const nlohmann::json& entityJson) {
    entt::entity entity = registry.create();

    // Loop over components

    for (auto& [compName, compJson] : entityJson.items()) {
        try {
            bool success = debug::MetaRegistry::EmplaceComponent(compName, registry, entity, compJson);
            if (!success) {
                LOG_WARN("EntityFactory::CreateEntity: Failed to emplace component '{}'", compName);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("EntityFactory::CreateEntity: Exception caught emplacing component '{}': {}", compName, e.what());
        }
    }

    // Deterministic StableId generation if omitted or failed to emplace
    if (!registry.all_of<StableId>(entity)) {
        StableId s;
        s.uuid = core::prng::next_random();
        registry.emplace<StableId>(entity, s);
    }

    return entity;
}

std::vector<entt::entity> EntityFactory::LoadPrefab(entt::registry& registry, const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("EntityFactory::LoadPrefab: Failed to open prefab file '{}'", filepath);
        return {};
    }

    nlohmann::json prefabJson;
    try {
        file >> prefabJson;
    } catch (const nlohmann::json::parse_error& e) {
        LOG_ERROR("EntityFactory::LoadPrefab: JSON parse error in '{}': {}", filepath, e.what());
        return {};
    }

    std::vector<entt::entity> entities;
    if (prefabJson.is_array()) {
        for (const auto& entityJson : prefabJson) {
            entities.push_back(CreateEntity(registry, entityJson));
        }
    } else if (prefabJson.is_object()) {
        entities.push_back(CreateEntity(registry, prefabJson));
    } else {
        LOG_ERROR("EntityFactory::LoadPrefab: Prefab JSON must be an array of entities or a single entity object.");
    }

    LOG_INFO("EntityFactory: Loaded {} entities from prefab '{}'", entities.size(), filepath);
    return entities;
}

} // namespace ecs