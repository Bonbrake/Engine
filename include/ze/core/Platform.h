#pragma once

#include <string>

// Replaces standard assert with ENGINE_ASSERT macro
#ifdef _DEBUG
#define ENGINE_ASSERT(x, ...) { if(!(x)) { LOG_CRITICAL("Assertion Failed: {0}", __VA_ARGS__); ::core::Platform::triggerBreakpoint(); } }
#else
#define ENGINE_ASSERT(x, ...)
#endif

namespace core {

class Platform {
public:
    static void initCrashHandler();
    static void triggerBreakpoint();
    static void writeMinidump(void* exceptionPointers);
    static void writeCrashSidecar(const std::string& info);
};

} // namespace core
