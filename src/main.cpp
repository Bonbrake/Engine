#include "core/Config.h"
#include "core/Engine.h"
#include "core/Logger.h"
#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
    try {
        // 1. Parse CLI arguments
        core::Config::get().parseCommandLine(argc, argv);
        
        // 2. Load settings
        core::Config::get().loadSettings();

        // 3. Initialize and run engine
        core::Engine engine;
        engine.run();

        if (core::Config::get().headless) {
            if (engine.verifyHeadlessInit()) {
                LOG_INFO("HEADLESS CI SMOKE TEST: SUCCESS");
                return 0;
            } else {
                LOG_CRITICAL("HEADLESS CI SMOKE TEST: FAILED");
                return 1;
            }
        }
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        if (msg.find("VK_ERROR_DEVICE_LOST") != std::string::npos) {
            if (core::Logger::get()) {
                LOG_CRITICAL("Vulkan Device Lost exception caught: {}. Clean exit triggered.", msg);
            } else {
                std::cerr << "Vulkan Device Lost: " << msg << std::endl;
            }
            return -4; // VK_ERROR_DEVICE_LOST exit code
        }
        if (core::Logger::get()) {
            LOG_CRITICAL("Engine terminated unexpectedly: {}", msg);
        } else {
            std::cerr << "Engine terminated unexpectedly: " << msg << std::endl;
        }
        return 1;
    } catch (const std::exception& e) {
        if (core::Logger::get()) {
            LOG_CRITICAL("Engine terminated unexpectedly: {}", e.what());
        } else {
            std::cerr << "Engine terminated unexpectedly: " << e.what() << std::endl;
        }
        return 1;
    }
    return 0;
}
