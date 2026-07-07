#include "Systems.h"
#include "Components.h"

namespace ecs {

void Systems::MovementSystem(entt::registry& reg, float dt) {
    auto view = reg.view<Position, const Velocity>();

    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        const auto& vel = view.get<Velocity>(entity);

        pos.value += vel.value * dt;
    }
}

} // namespace ecs