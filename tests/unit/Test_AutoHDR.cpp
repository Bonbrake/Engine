#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <volk.h>
#include <vector>
#include <algorithm>

#include "ze/core/Config.h"
#include "ze/render/Swapchain.h"

// Unit test verifying Easy Automatic HDR configuration, display capabilities,
// luminance limits, and surface format negotiation priority logic.

TEST_CASE("Auto-HDR: Default configuration and boundaries", "[hdr][config]") {
    const auto& cfg = core::Config::get();

    // Zero-friction default: Auto mode (2)
    REQUIRE(cfg.hdrMode == 2);

    // Standard OLED/Mini-LED target defaults
    REQUIRE(cfg.hdrPeakNits == Catch::Approx(1000.0f));
    REQUIRE(cfg.hdrPaperWhiteNits == Catch::Approx(200.0f));

    // Paper-white must never exceed peak nits
    REQUIRE(cfg.hdrPaperWhiteNits <= cfg.hdrPeakNits);

    // Paper-white must stay within comfortable SDR range (80-500 nits) to prevent eye burn
    REQUIRE(cfg.hdrPaperWhiteNits >= 80.0f);
    REQUIRE(cfg.hdrPaperWhiteNits <= 500.0f);
}

TEST_CASE("Auto-HDR: Display capabilities structure defaults", "[hdr][capabilities]") {
    render::HdrDisplayCapabilities caps{};

    REQUIRE_FALSE(caps.hdrSupported);
    REQUIRE_FALSE(caps.hdrActive);
    REQUIRE(caps.format == VK_FORMAT_UNDEFINED);
    REQUIRE(caps.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    REQUIRE(caps.peakLuminanceNits == Catch::Approx(1000.0f));
    REQUIRE(caps.paperWhiteNits == Catch::Approx(200.0f));
    REQUIRE(caps.minLuminanceNits == Catch::Approx(0.0001f));
}

TEST_CASE("Auto-HDR: Surface format priority selection simulation", "[hdr][negotiation]") {
    // Helper function reproducing the Swapchain format negotiation algorithm
    auto negotiateFormat = [](int mode, const std::vector<VkSurfaceFormatKHR>& formats,
                              VkSurfaceFormatKHR& outChosen, bool& outHdrActive) {
        outHdrActive = false;
        outChosen = {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

        if (formats.empty() || mode == 0) {
            return; // Force SDR or no formats
        }

        bool hdrSupported = false;
        for (const auto& sf : formats) {
            if (sf.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT ||
                sf.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT) {
                hdrSupported = true;
                break;
            }
        }

        if (!hdrSupported) return;

        // Priority 1: 10-bit HDR10 (ST.2084 PQ)
        for (const auto& sf : formats) {
            if (sf.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT &&
                (sf.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 || sf.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32)) {
                outChosen = sf;
                outHdrActive = true;
                return;
            }
        }

        // Priority 2: scRGB Linear FP16 (Extended sRGB Linear)
        for (const auto& sf : formats) {
            if (sf.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT &&
                sf.format == VK_FORMAT_R16G16B16A16_SFLOAT) {
                outChosen = sf;
                outHdrActive = true;
                return;
            }
        }

        // Priority 3: Any HDR10 format
        for (const auto& sf : formats) {
            if (sf.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT) {
                outChosen = sf;
                outHdrActive = true;
                return;
            }
        }
    };

    SECTION("SDR display only") {
        std::vector<VkSurfaceFormatKHR> sdrFormats = {
            {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
        };
        VkSurfaceFormatKHR chosen{};
        bool active = false;
        negotiateFormat(2, sdrFormats, chosen, active);
        REQUIRE_FALSE(active);
        REQUIRE(chosen.format == VK_FORMAT_R8G8B8A8_UNORM);
        REQUIRE(chosen.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    }

    SECTION("HDR display with 10-bit HDR10 and scRGB") {
        std::vector<VkSurfaceFormatKHR> hdrFormats = {
            {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            {VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT},
            {VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_COLOR_SPACE_HDR10_ST2084_EXT}
        };
        VkSurfaceFormatKHR chosen{};
        bool active = false;

        // Auto mode should pick Priority 1 (HDR10 A2B10G10R10)
        negotiateFormat(2, hdrFormats, chosen, active);
        REQUIRE(active);
        REQUIRE(chosen.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32);
        REQUIRE(chosen.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT);

        // Force SDR (mode 0) should ignore HDR even when present
        negotiateFormat(0, hdrFormats, chosen, active);
        REQUIRE_FALSE(active);
        REQUIRE(chosen.format == VK_FORMAT_R8G8B8A8_UNORM);
        REQUIRE(chosen.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    }

    SECTION("HDR display with scRGB only") {
        std::vector<VkSurfaceFormatKHR> scRgbFormats = {
            {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            {VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT}
        };
        VkSurfaceFormatKHR chosen{};
        bool active = false;
        negotiateFormat(2, scRgbFormats, chosen, active);
        REQUIRE(active);
        REQUIRE(chosen.format == VK_FORMAT_R16G16B16A16_SFLOAT);
        REQUIRE(chosen.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT);
    }
}
