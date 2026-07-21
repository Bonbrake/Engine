#pragma once

#include <vector>
#include <string>
#include "../core/Logger.h"

namespace core {

class TeardownTracker {
public:
    enum class Stage {
        None,
        JobSystem,
        SDL,
        VulkanInstance,
        Device,
        Swapchain,
        AssetManager,
        MaterialSystem,
        ImGui,
        Count
    };

    static void RegisterInit(Stage stage, const char* name) {
        auto& self = Get();
        self.initOrder_.push_back(stage);
        self.names_[static_cast<int>(stage)] = name;
        LOG_INFO("RAII Init: {}", name);
    }

    static void RegisterShutdown(Stage stage) {
        auto& self = Get();
        if (self.initOrder_.empty()) {
            LOG_ERROR("RAII Shutdown out of order: Shutting down {} but no systems initialized", self.names_[static_cast<int>(stage)]);
            return;
        }
        Stage expected = self.initOrder_.back();
        if (expected != stage) {
            LOG_ERROR("RAII Teardown Order Violation! Expected shutdown of: {}, but got: {}", 
                      self.names_[static_cast<int>(expected)], self.names_[static_cast<int>(stage)]);
        } else {
            LOG_INFO("RAII Shutdown: {}", self.names_[static_cast<int>(stage)]);
            self.initOrder_.pop_back();
        }
    }

private:
    static TeardownTracker& Get() {
        static TeardownTracker instance;
        return instance;
    }
    
    std::vector<Stage> initOrder_;
    std::string names_[static_cast<int>(Stage::Count)];
};

} // namespace core
