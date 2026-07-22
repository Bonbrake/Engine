#include "ze/ui/HUD.h"
#include "ze/ui/MSDFText.h"
#include "ze/debug/ImGuiOverlay.h"
#include "ze/core/Time.h"
#include <imgui.h>
#include <algorithm>
#include <sstream>

namespace ui {

void HUD::update(float health, float maxHealth, float stamina, float maxStamina,
                 float bodyTemp, float hunger, float thirst,
                 float playerYaw, bool isInCombat) {
    // Update health bar
    healthBar_.value = health / std::max(maxHealth, 1.0f);
    healthBar_.color = (health / std::max(maxHealth, 1.0f) > 0.3f)
        ? glm::vec3(0.2f, 0.8f, 0.2f)
        : glm::vec3(0.9f, 0.2f, 0.1f);

    // Update stamina bar
    staminaBar_.value = stamina / std::max(maxStamina, 1.0f);
    staminaBar_.color = glm::vec3(0.2f, 0.5f, 0.9f);

    // Update compass
    compass_.playerYaw = playerYaw;

    // Update notifications (decay timers)
    notifications_.erase(
        std::remove_if(notifications_.begin(), notifications_.end(),
            [](Notification& n) { n.remaining -= 1.0f / 60.0f; return n.remaining <= 0.0f; }),
        notifications_.end());
}

void HUD::addNotification(const std::string& text, float duration, glm::vec3 color) {
    Notification n;
    n.text = text;
    n.remaining = duration;
    n.color = color;
    notifications_.push_back(n);
}

void HUD::addDebugText(const std::string& text) {
    if (debugLines_.size() > 20) {
        debugLines_.erase(debugLines_.begin());
    }
    debugLines_.push_back(text);
}

void HUD::render() const {
    if (!visible_) return;

    // Use ImGui for HUD rendering (existing overlay infrastructure)
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);
    ImGui::Begin("HUD", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoInputs);

    // Health bar
    renderHealthBar(healthBar_);

    // Stamina bar
    renderHealthBar(staminaBar_);

    // Compass (simplified)
    renderCompass(compass_);

    // Notifications (centered, fading)
    ImGui::SetCursorPosY(60);
    for (const auto& n : notifications_) {
        float alpha = std::min(1.0f, n.remaining);
        ImGui::TextColored(
            ImVec4(n.color.r, n.color.g, n.color.b, alpha),
            "%s", n.text.c_str());
    }

    // Debug overlay
    if (debugVisible_) {
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);
        for (const auto& l : debugLines_) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", l.c_str());
        }
    }

    ImGui::End();
}

void HUD::renderHealthBar(const HUDHealthBar& bar) const {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 pos = ImVec2(bar.pos.x * ImGui::GetIO().DisplaySize.x,
                        bar.pos.y * ImGui::GetIO().DisplaySize.y);
    ImVec2 size = ImVec2(bar.width * ImGui::GetIO().DisplaySize.x,
                          bar.height * ImGui::GetIO().DisplaySize.y);

    // Background
    draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                        IM_COL32(40, 40, 40, 200));

    // Fill
    float fillWidth = size.x * std::max(0.0f, std::min(1.0f, bar.value));
    draw->AddRectFilled(pos, ImVec2(pos.x + fillWidth, pos.y + size.y),
                        IM_COL32(
                            static_cast<int>(bar.color.r * 255),
                            static_cast<int>(bar.color.g * 255),
                            static_cast<int>(bar.color.b * 255),
                            200));
}

void HUD::renderCompass(const HUDCompass& compass) const {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 center = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                            ImGui::GetIO().DisplaySize.y * 0.9f);
    float r = compass.radius * ImGui::GetIO().DisplaySize.x;

    // Compass arc
    draw->PathArcTo(center, r, -ImGui::GetIO().DisplaySize.x * 0.5f,
                    ImGui::GetIO().DisplaySize.x * 0.5f, 32);
    draw->PathStroke(IM_COL32(255, 255, 255, 100), false, 2.0f);

    // Cardinal directions
    draw->AddText(ImVec2(center.x - 10, center.y - r - 20),
                  IM_COL32(255, 255, 255, 200), "N");
    draw->AddText(ImVec2(center.x + r + 5, center.y - 8),
                  IM_COL32(255, 255, 255, 150), "E");
    draw->AddText(ImVec2(center.x - 10, center.y + r + 5),
                  IM_COL32(255, 255, 255, 150), "S");
    draw->AddText(ImVec2(center.x - r - 20, center.y - 8),
                  IM_COL32(255, 255, 255, 150), "W");
}

void HUD::renderText(const HUDTextElement& elem) const {
    // Placeholder for MSDF text rendering (future: integrate MSDFPipeline)
    ImVec2 pos(elem.pos.x * ImGui::GetIO().DisplaySize.x,
               elem.pos.y * ImGui::GetIO().DisplaySize.y);
    ImGui::SetCursorPos(pos);
    ImGui::TextColored(ImVec4(elem.color.r, elem.color.g, elem.color.b, 1),
                       "%s", elem.text.c_str());
}

} // namespace ui
