#pragma once

#ifdef JPH_DEBUG_RENDERER

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#include <glm/glm.hpp>
#include <vector>

namespace physics {

class PhysicsDebugRenderer final : public JPH::DebugRendererSimple {
public:
    PhysicsDebugRenderer();
    virtual ~PhysicsDebugRenderer() override = default;

    virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
    virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override;
    virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) override;

    struct Line {
        glm::vec3 from;
        glm::vec3 to;
        uint32_t color; // ImGui color (ABGR)
    };

    void clearLines();
    const std::vector<Line>& getLines() const { return lines_; }

private:
    std::vector<Line> lines_;
};

} // namespace physics

#endif // JPH_DEBUG_RENDERER
