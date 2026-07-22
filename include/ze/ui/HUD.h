#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ui {

// HUD element types
struct HUDHealthBar {
    glm::vec2 pos;       // screen-space position (0..1 normalized)
    float width = 0.15f;
    float height = 0.02f;
    float value = 1.0f;  // 0..1
    glm::vec3 color = {0.2f, 0.8f, 0.2f};
};

struct HUDTextElement {
    std::string text;
    glm::vec2 pos;
    float size = 16.0f;
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    bool centered = false;
};

struct HUDCompass {
    glm::vec2 pos;
    float radius = 0.08f;
    float playerYaw = 0.0f;
    std::vector<std::pair<float, std::string>> markers; // angle, label
};

// Minimal HUD: health bar, compass, notifications, debug text
class HUD {
public:
    HUD() = default;
    ~HUD() = default;

    // Per-tick update from game state
    void update(float health, float maxHealth, float stamina, float maxStamina,
                float bodyTemp, float hunger, float thirst,
                float playerYaw, bool isInCombat);

    // Notification queue
    void addNotification(const std::string& text, float duration = 3.0f,
                         glm::vec3 color = {1.0f, 1.0f, 1.0f});
    void addDebugText(const std::string& text);

    // Render submission: called once per frame from ImGui/render overlay
    void render() const;

    // Visibility
    void setVisible(bool v) { visible_ = v; }
    bool isVisible() const { return visible_; }
    void setDebugVisible(bool v) { debugVisible_ = v; }

    // Configuration
    void setHealthBarPos(glm::vec2 pos) { healthBar_.pos = pos; }
    void setCompassPos(glm::vec2 pos) { compass_.pos = pos; }

private:
    HUDHealthBar healthBar_;
    HUDHealthBar staminaBar_;
    HUDCompass compass_;
    bool visible_ = true;
    bool debugVisible_ = false;

    struct Notification {
        std::string text;
        float remaining = 0.0f;
        glm::vec3 color;
    };
    std::vector<Notification> notifications_;
    std::vector<std::string> debugLines_;

    void renderHealthBar(const HUDHealthBar& bar) const;
    void renderCompass(const HUDCompass& compass) const;
    void renderText(const HUDTextElement& elem) const;
};

} // namespace ui
