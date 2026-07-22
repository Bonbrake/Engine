#pragma once
#include <string>
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace ui {

// Multi-channel signed-distance-field text rendering
// Wraps MSDFPipeline for runtime text:
//   - Font atlas: compile-time generated MSDF glyph cache
//   - Layout: simple line-based text formatting
//   - Styling: color, size, outline, drop shadow

struct TextGlyph {
    uint32_t codepoint;
    float advance;       // horizontal advance in pixels
    glm::vec2 bearing;   // offset from baseline
    glm::vec2 uv0, uv1;  // atlas UV coordinates
    glm::vec2 size;      // glyph pixel size
};

struct TextLayout {
    std::vector<uint32_t> codepoints;
    std::vector<glm::vec2> positions; // screen-space positions per glyph
    glm::vec2 boundingBox;           // total rendered size in pixels
    float fontSize = 16.0f;
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float outlineWidth = 0.0f;
    glm::vec3 outlineColor = {0.0f, 0.0f, 0.0f};
    bool dropShadow = false;
};

class MSDFText {
public:
    MSDFText() = default;
    ~MSDFText() = default;

    // Initialize with font data (atlas texture + glyph metrics)
    // In production, font is compiled as embedded binary
    bool initialize(const uint8_t* fontData, size_t fontDataSize);

    // Layout text to screen coords
    TextLayout layout(const std::string& text, float x, float y, float fontSize);
    TextLayout layoutWrapped(const std::string& text, float x, float y,
                             float maxWidth, float fontSize);

    // Render prepared layout
    void render(const TextLayout& layout);

    // Glyph cache
    const TextGlyph* getGlyph(uint32_t codepoint, float fontSize);
    void preCacheGlyphs(const std::string& text, float fontSize);

    // Metrics
    float measureWidth(const std::string& text, float fontSize) const;
    float measureHeight(float fontSize) const { return fontSize * 1.2f; }

private:
    struct FontFace {
        std::unordered_map<uint32_t, TextGlyph> glyphs;
        float lineHeight = 1.2f;
        float ascender = 0.8f;
        float descender = -0.2f;
    };

    FontFace font_;
    bool initialized_ = false;

    // Internal: rasterize glyph to MSDF atlas via MSDFPipeline
    bool rasterizeGlyph(uint32_t codepoint, float fontSize);
};

} // namespace ui
