#pragma once

#include <glm/glm.hpp>

namespace ecs {

struct Position {
    glm::vec3 value;
};

struct Velocity {
    glm::vec3 value;
};

struct Renderable {
    // Empty tag component for M1 (indicates this entity has a mesh we should draw)
};

} // namespace ecs