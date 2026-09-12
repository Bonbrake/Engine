#pragma once

#include <cstdint>
#include <string>
#include "ze/slm/SLMClient.h"

namespace slm {

// Engine-wide SLM singleton
// Uses a function-local static to avoid ODR violations when SLM.h
// is included from multiple translation units
inline SLMClient& getClient() {
    static SLMClient instance;
    return instance;
}

namespace SLM {

    inline bool Init(const SLMConfig& config = SLMConfig()) {
        return getClient().initialize(config);
    }

    inline void Shutdown() {
        getClient().shutdown();
    }

    inline void Submit(uint64_t entityId, const char* tag, const std::string& promptFields, SLMMode mode = SLMMode::Fast) {
        getClient().submitRequest(entityId, tag ? tag : "DEFAULT", promptFields, mode);
    }

    template <typename ApplyFn>
    inline void Update(ApplyFn&& apply) {
        getClient().updateMainThread(apply);
    }

    template <typename ApplyFn>
    inline void UpdateEx(ApplyFn&& apply) {
        getClient().updateMainThreadEx(apply);
    }

    inline bool IsAvailable() {
        return getClient().isAvailable();
    }

    inline const SLMStats& GetStats() {
        return getClient().getStats();
    }

    inline void SetBackend(BackendType type, const std::string& endpoint = "", const std::string& apiKey = "") {
        getClient().setBackend(type, endpoint, apiKey);
    }

} // namespace SLM

} // namespace slm
