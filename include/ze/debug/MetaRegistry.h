#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace debug {

class MetaRegistry {
public:
    static void Initialize();
    static bool EmplaceComponent(const std::string& name, entt::registry& registry, entt::entity entity, const nlohmann::json& json);
};

} // namespace debug
