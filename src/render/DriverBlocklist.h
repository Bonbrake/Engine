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
        std::vector<BlocklistedDriver> blocklist = {
            // Documented extension point: Add blocklisted hardware configurations here
            // Example: { 0x10DE, 0x1F08, "555.00", "Simulated driver issue" }
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
