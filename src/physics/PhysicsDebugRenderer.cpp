#include "ze/physics/PhysicsDebugRenderer.h"

#ifdef JPH_DEBUG_RENDERER

namespace physics {

PhysicsDebugRenderer::PhysicsDebugRenderer() {
    Initialize();
}

void PhysicsDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) {
    uint32_t c = inColor.GetUInt32();
    // Jolt color is usually RGBA. ImGui is usually ABGR (or whatever ImGui::GetColorU32 takes natively)
    // Actually Jolt's Color::GetUInt32() returns a uint32_t with R in the lowest byte if Little Endian?
    // Let's just use ImGui ABGR format which is typically: (A << 24) | (B << 16) | (G << 8) | R
    uint8_t r = inColor.r;
    uint8_t g = inColor.g;
    uint8_t b = inColor.b;
    uint8_t a = inColor.a;
    uint32_t imColor = (a << 24) | (b << 16) | (g << 8) | r;

    lines_.push_back({
        glm::vec3(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()),
        glm::vec3(inTo.GetX(), inTo.GetY(), inTo.GetZ()),
        imColor
    });
}

void PhysicsDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) {
    // Draw edges of the triangle
    DrawLine(inV1, inV2, inColor);
    DrawLine(inV2, inV3, inColor);
    DrawLine(inV3, inV1, inColor);
}

void PhysicsDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) {
    // Not implemented
}

void PhysicsDebugRenderer::clearLines() {
    lines_.clear();
}

} // namespace physics

#endif // JPH_DEBUG_RENDERER