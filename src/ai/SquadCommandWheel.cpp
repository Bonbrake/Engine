#include "ze/ai/SquadCommandWheel.h"
#include "ze/ai/ResponseSystem.h"
#include <cmath>
#include <limits>

namespace ze::ai {

SquadCommandWheel::SquadCommandWheel() {
}

void SquadCommandWheel::registerSquadMember(uint64_t entityId, const std::string& name) {
    for (auto& m : m_squad) {
        if (m.entityId == entityId) {
            m.name = name;
            m.isAlive = true;
            return;
        }
    }
    SquadMember member;
    member.entityId = entityId;
    member.name = name;
    member.isAlive = true;
    member.currentOrder = SquadDirective::None;
    m_squad.push_back(member);
}

void SquadCommandWheel::updateMemberPosition(uint64_t entityId, float x, float y, float z) {
    for (auto& m : m_squad) {
        if (m.entityId == entityId) {
            m.position[0] = x;
            m.position[1] = y;
            m.position[2] = z;
            return;
        }
    }
}

void SquadCommandWheel::openWheel() {
    m_isOpen = true;
    m_selectedDirective = SquadDirective::None;
}

void SquadCommandWheel::closeWheel() {
    m_isOpen = false;
    m_selectedDirective = SquadDirective::None;
}

void SquadCommandWheel::updateStickInput(float stickX, float stickY) {
    if (!m_isOpen) {
        m_selectedDirective = SquadDirective::None;
        return;
    }

    float magSq = stickX * stickX + stickY * stickY;
    if (magSq < (0.35f * 0.35f)) {
        // Inside deadzone
        m_selectedDirective = SquadDirective::None;
        return;
    }

    // Calculate angle in degrees [0, 360)
    // stickX: right positive, stickY: up positive
    float angleRad = std::atan2(stickY, stickX);
    float angleDeg = angleRad * (180.0f / 3.14159265358979323846f);
    if (angleDeg < 0.0f) {
        angleDeg += 360.0f;
    }

    // 6 sectors of 60 degrees centered at 0, 60, 120, 180, 240, 300
    // Offset by +30 degrees so sector 0 covers [330, 30)
    float sectorAngle = angleDeg + 30.0f;
    if (sectorAngle >= 360.0f) sectorAngle -= 360.0f;
    int sector = static_cast<int>(sectorAngle / 60.0f);

    switch (sector) {
        case 0: m_selectedDirective = SquadDirective::HoldPosition; break;
        case 1: m_selectedDirective = SquadDirective::BreachAndClear; break;
        case 2: m_selectedDirective = SquadDirective::BarricadeWindow; break;
        case 3: m_selectedDirective = SquadDirective::FocusFire; break;
        case 4: m_selectedDirective = SquadDirective::ScavengeSupplies; break;
        case 5: m_selectedDirective = SquadDirective::FallBack; break;
        default: m_selectedDirective = SquadDirective::HoldPosition; break;
    }
}

uint64_t SquadCommandWheel::findBestSquadMember(const float targetPos[3]) {
    uint64_t bestId = 0;
    float bestDistSq = std::numeric_limits<float>::max();

    for (const auto& m : m_squad) {
        if (!m.isAlive) continue;
        float dx = m.position[0] - targetPos[0];
        float dy = m.position[1] - targetPos[1];
        float dz = m.position[2] - targetPos[2];
        float distSq = dx * dx + dy * dy + dz * dz;
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestId = m.entityId;
        }
    }
    return bestId;
}

bool SquadCommandWheel::confirmOrder(uint64_t targetEntityId, const float targetPos[3], ResponseSystem* responseSystem) {
    (void)targetEntityId;
    if (!m_isOpen || m_selectedDirective == SquadDirective::None) {
        return false;
    }

    uint64_t assignedMemberId = findBestSquadMember(targetPos);
    if (assignedMemberId == 0 && !m_squad.empty()) {
        assignedMemberId = m_squad.front().entityId;
    }

    if (assignedMemberId != 0) {
        for (auto& m : m_squad) {
            if (m.entityId == assignedMemberId) {
                m.currentOrder = m_selectedDirective;
                break;
            }
        }

        if (responseSystem) {
            CriteriaQuery q;
            q.speakerEntityId = assignedMemberId;
            q.isNpc = true;
            q.speechConcept = SpeechConcept::SquadCommandAcknowledged;
            q.healthFraction = 1.0f;
            q.directorIntensity = 0.5f;
            q.distanceToPlayer = 3.0f;
            q.speakerWorldPos = glm::vec3(targetPos[0], targetPos[1], targetPos[2]);

            DispatchedSpeechResponse resp;
            responseSystem->queryResponse(q, resp);
        }
    }

    closeWheel();
    return true;
}

const char* SquadCommandWheel::getDirectiveName(SquadDirective directive) {
    switch (directive) {
        case SquadDirective::HoldPosition: return "Hold Position";
        case SquadDirective::BreachAndClear: return "Breach & Clear";
        case SquadDirective::BarricadeWindow: return "Barricade Window";
        case SquadDirective::FocusFire: return "Focus Fire";
        case SquadDirective::ScavengeSupplies: return "Scavenge Supplies";
        case SquadDirective::FallBack: return "Fall Back";
        default: return "None";
    }
}

const char* SquadCommandWheel::getDirectiveBarkConcept(SquadDirective directive) {
    switch (directive) {
        case SquadDirective::HoldPosition: return "order_ack_hold";
        case SquadDirective::BreachAndClear: return "order_ack_breach";
        case SquadDirective::BarricadeWindow: return "order_ack_barricade";
        case SquadDirective::FocusFire: return "order_ack_focusfire";
        case SquadDirective::ScavengeSupplies: return "order_ack_scavenge";
        case SquadDirective::FallBack: return "order_ack_fallback";
        default: return "order_ack_generic";
    }
}

} // namespace ze::ai
