#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ze::ai {

class ResponseSystem;

enum class SquadDirective : uint8_t {
    None = 0,
    HoldPosition,
    BreachAndClear,
    BarricadeWindow,
    FocusFire,
    ScavengeSupplies,
    FallBack,
    Count
};

struct SquadMember {
    uint64_t entityId = 0;
    std::string name;
    float position[3] = {0.0f, 0.0f, 0.0f};
    bool isAlive = true;
    SquadDirective currentOrder = SquadDirective::None;
};

class SquadCommandWheel {
public:
    SquadCommandWheel();
    ~SquadCommandWheel() = default;

    void registerSquadMember(uint64_t entityId, const std::string& name);
    void updateMemberPosition(uint64_t entityId, float x, float y, float z);

    // Radial Menu Activation (Hold LB / L1)
    void openWheel();
    void closeWheel();
    bool isOpen() const { return m_isOpen; }

    // Analog Stick Selection (Deadzone = 0.35f, 6 radial slices of 60 degrees)
    void updateStickInput(float stickX, float stickY);
    SquadDirective getSelectedDirective() const { return m_selectedDirective; }

    // Execute order targeted at world coordinates/entity
    bool confirmOrder(uint64_t targetEntityId, const float targetPos[3], ResponseSystem* responseSystem);

    static const char* getDirectiveName(SquadDirective directive);
    static const char* getDirectiveBarkConcept(SquadDirective directive);

    const std::vector<SquadMember>& getSquadMembers() const { return m_squad; }

private:
    bool m_isOpen = false;
    SquadDirective m_selectedDirective = SquadDirective::None;
    std::vector<SquadMember> m_squad;

    uint64_t findBestSquadMember(const float targetPos[3]);
};

} // namespace ze::ai
