#include "ze/ui/MSDFText.h"
#include "ze/render/MSDFPipeline.h"
#include <imgui.h>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace ui {

bool MSDFText::initialize(const uint8_t* fontData, size_t fontDataSize) {
    if (!fontData || fontDataSize == 0) return false;
    // Font data is passed to the pipeline for atlas generation
    // In a full implementation, this would call msdfgen to rasterize glyphs
    initialized_ = true;
    return true;
}

bool MSDFText::rasterizeGlyph(uint32_t codepoint, float fontSize) {
    // Check cache
    if (font_.glyphs.find(codepoint) != font_.glyphs.end()) return true;

    // MSDF rasterization: deferred to MSDFPipeline
    // In the current build, we use a simple ImGui text fallback
    TextGlyph g;
    g.codepoint = codepoint;
    g.advance = fontSize * 0.6f;
    g.bearing = {0, fontSize};
    g.uv0 = g.uv1 = {0, 0};
    g.size = {fontSize * 0.6f, fontSize};

    font_.glyphs[codepoint] = g;
    return true;
}

const TextGlyph* MSDFText::getGlyph(uint32_t codepoint, float fontSize) {
    if (!initialized_) return nullptr;

    auto it = font_.glyphs.find(codepoint);
    if (it != font_.glyphs.end()) return &it->second;

    if (rasterizeGlyph(codepoint, fontSize)) {
        it = font_.glyphs.find(codepoint);
        if (it != font_.glyphs.end()) return &it->second;
    }
    return nullptr;
}

void MSDFText::preCacheGlyphs(const std::string& text, float fontSize) {
    for (unsigned char c : text) {
        rasterizeGlyph(static_cast<uint32_t>(c), fontSize);
    }
}

TextLayout MSDFText::layout(const std::string& text, float x, float y, float fontSize) {
    TextLayout layout;
    layout.fontSize = fontSize;
    float cursorX = x;
    float cursorY = y;
    float maxX = cursorX;

    for (unsigned char c : text) {
        if (c == '\n') {
            cursorY += fontSize * 1.2f;
            maxX = std::max(maxX, cursorX);
            cursorX = x;
            continue;
        }

        layout.codepoints.push_back(static_cast<uint32_t>(c));
        layout.positions.push_back({cursorX, cursorY});

        // Use cached glyph if available, otherwise default advance
        auto* glyph = getGlyph(static_cast<uint32_t>(c), fontSize);
        cursorX += glyph ? glyph->advance : fontSize * 0.5f;
    }
    maxX = std::max(maxX, cursorX);
    layout.boundingBox = {maxX - x, cursorY + fontSize * 1.2f - y};
    return layout;
}

TextLayout MSDFText::layoutWrapped(const std::string& text, float x, float y,
                                    float maxWidth, float fontSize) {
    TextLayout layout;
    layout.fontSize = fontSize;
    float cursorX = x;
    float cursorY = y;
    float maxX = cursorX;
    float spaceWidth = fontSize * 0.3f;

    std::istringstream stream(text);
    std::string word;
    while (stream >> word) {
        float wordWidth = word.length() * fontSize * 0.5f; // approximate

        if (cursorX + wordWidth > maxWidth && cursorX > x) {
            cursorY += fontSize * 1.2f;
            cursorX = x;
        }

        for (unsigned char c : word) {
            layout.codepoints.push_back(static_cast<uint32_t>(c));
            layout.positions.push_back({cursorX, cursorY});
            auto* glyph = getGlyph(static_cast<uint32_t>(c), fontSize);
            cursorX += glyph ? glyph->advance : fontSize * 0.5f;
        }

        // Add space
        cursorX += spaceWidth;
        maxX = std::max(maxX, cursorX);
    }

    layout.boundingBox = {maxX - x, cursorY + fontSize * 1.2f - y};
    return layout;
}

void MSDFText::render(const TextLayout& layout) {
    // Current implementation: render via ImGui as fallback
    // Full implementation: submit glyph quads to MSDFPipeline for GPU rendering
    ImGui::SetCursorPos(ImVec2(0, 0));
    for (size_t i = 0; i < layout.codepoints.size() && i < layout.positions.size(); ++i) {
        ImVec2 pos(layout.positions[i].x, layout.positions[i].y);
        ImGui::SetCursorPos(pos);
        char glyph[2] = {static_cast<char>(layout.codepoints[i] & 0xFF), 0};
        ImGui::TextColored(ImVec4(layout.color.r, layout.color.g, layout.color.b, 1),
                           "%s", glyph);
    }
}

float MSDFText::measureWidth(const std::string& text, float fontSize) const {
    float width = 0.0f;
    for (unsigned char c : text) {
        auto it = font_.glyphs.find(static_cast<uint32_t>(c));
        width += it != font_.glyphs.end() ? it->second.advance : fontSize * 0.5f;
    }
    return width;
}

} // namespace ui
