#pragma once

#include <entt/entt.hpp>

namespace ecs {

class Systems {
public:
    static void MovementSystem(entt::registry& reg, float dt);
};

} // namespace ecs