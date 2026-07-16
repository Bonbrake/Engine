#pragma once

#include <string>
#include <vector>
#include <volk.h>
#include "../core/Logger.h"

namespace render {

struct DriverWorkaround {
    std::string name;
    bool enabled = false;
};

struct BlocklistedDriver {
    uint32_t vendorId;
    uint32_t deviceId;
    std::string driverVersionRange;
    std::string reason;
};

class DriverBlocklist {
public:
    static bool IsDeviceBlocklisted(uint32_t vendorId, uint32_t deviceId, const char* deviceName) {
        // Seed blocklist — REAL, documented Vulkan driver hazards only.
        // Extend from actual crash reports (crash_dump.dmp / crash_sidecar.txt), never from
        // guessed device IDs. vendorID: NVIDIA=0x10DE, AMD=0x1002, Intel=0x8086.
        // deviceID values below are real PCI IDs for the named parts.
        std::vector<BlocklistedDriver> blocklist = {
            // Intel UHD 630 (0x3E9B, Gen9.5) — documented dynamic-rendering /
            // conservative-rasterization edge cases on older Windows Vulkan ICDs.
            { 0x8086, 0x3E9B, "*", "Intel Gen9.5 dynamic-render edge cases; prefer Tier-0 fallback path" },
            // Template (uncomment + verify against a real crash report before enabling):
            // { 0x1002, 0x73FF, "*", "AMD RX 6700 XT: VK_EXT_descriptor_buffer hazard on early Windows ICD" },
            // { 0x10DE, 0x1F08, "*", "NVIDIA TU106: <documented issue + source>" },
        };

        for (const auto& entry : blocklist) {
            if (entry.vendorId == vendorId && entry.deviceId == deviceId) {
                LOG_WARN("GPU is blocklisted: vendorId=0x{:X}, deviceId=0x{:X}, name={}. Reason: {}", 
                         vendorId, deviceId, deviceName, entry.reason);
                return true;
            }
        }
        return false;
    }

    static std::vector<DriverWorkaround> GetWorkarounds(uint32_t vendorId, uint32_t deviceId) {
        std::vector<DriverWorkaround> workarounds = {
            { "DisableComputeCulling", false }
        };

        // Example workaround: Disable compute culling on Intel GPUs due to known driver culling hazards
        if (vendorId == 0x8086) {
            for (auto& wa : workarounds) {
                if (wa.name == "DisableComputeCulling") {
                    wa.enabled = true;
                    LOG_INFO("Driver Workaround enabled for Intel GPU: DisableComputeCulling");
                }
            }
        }

        return workarounds;
    }
};

} // namespace render
