#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <utility>
#include <chrono>

namespace slm {

// Inference mode controlling dynamic reasoning vs fast execution in a single model
enum class SLMMode : uint8_t {
    Fast = 0,   // Sub-300ms direct generation: bypasses <think> CoT for instantaneous response
    Think = 1   // Deliberate reasoning: generates full <think> CoT for deep tactical planning / lore
};

// Request struct submitted by EnTT ECS components on the main thread
struct SLMRequest {
    uint64_t entityId = 0;       // entt::entity handle encoded as uint64_t
    std::string tag;            // System tag e.g. "M13-EXT-25", "RADIO", "LORE"
    std::string promptFields;   // Filled prompt string or serialized json input
    SLMMode mode = SLMMode::Fast; // Fast direct execution vs Deliberate thinking
    std::chrono::steady_clock::time_point submitTime; // Timestamp for timeout tracking
};

// Result struct populated by worker thread upon inference completion
struct SLMResult {
    uint64_t entityId = 0;       // entt::entity handle target
    std::string text;           // Generated text output / JSON string
    std::string reasoning;      // Internal chain-of-thought (<think>) extracted for telemetry
    bool success = true;
    float latencyMs = 0.0f;     // Inference latency for telemetry
};

// 256-element lock-free Single-Producer Single-Consumer request queue
// Producer: main render thread | Consumer: SLM worker thread
class alignas(64) SPSCRequestQueue {
public:
    static constexpr uint32_t CAPACITY = 256;
    static constexpr uint32_t MASK = CAPACITY - 1;

    bool Push(uint64_t entityId, std::string tag, std::string promptFields, SLMMode mode = SLMMode::Fast) {
        uint32_t w = m_writeHead.load(std::memory_order_relaxed);
        uint32_t next = (w + 1) & MASK;
        if (next == m_readHead.load(std::memory_order_acquire)) {
            return false; // Queue full, drop request for this frame
        }
        m_ring[w].entityId = entityId;
        m_ring[w].tag = std::move(tag);
        m_ring[w].promptFields = std::move(promptFields);
        m_ring[w].mode = mode;
        m_ring[w].submitTime = std::chrono::steady_clock::now();
        m_writeHead.store(next, std::memory_order_release);
        return true;
    }

    bool Pop(SLMRequest& out) {
        uint32_t r = m_readHead.load(std::memory_order_relaxed);
        if (r == m_writeHead.load(std::memory_order_acquire)) {
            return false; // Queue empty
        }
        out = std::move(m_ring[r]);
        m_readHead.store((r + 1) & MASK, std::memory_order_release);
        return true;
    }

    uint32_t size() const {
        uint32_t w = m_writeHead.load(std::memory_order_acquire);
        uint32_t r = m_readHead.load(std::memory_order_acquire);
        return (w - r) & MASK;
    }

private:
    alignas(64) SLMRequest m_ring[CAPACITY];
    alignas(64) std::atomic<uint32_t> m_writeHead{0};
    alignas(64) std::atomic<uint32_t> m_readHead{0};
};

// 256-element lock-free Single-Producer Single-Consumer result queue
// Producer: SLM worker thread | Consumer: main render thread
class alignas(64) SLMResultQueue {
public:
    static constexpr uint32_t CAPACITY = 256;
    static constexpr uint32_t MASK = CAPACITY - 1;

    bool Push(uint64_t entityId, std::string text, bool success = true, float latencyMs = 0.0f, std::string reasoning = "") {
        uint32_t w = m_writeHead.load(std::memory_order_relaxed);
        uint32_t next = (w + 1) & MASK;
        if (next == m_readHead.load(std::memory_order_acquire)) {
            return false; // Consumer stalled, drop result
        }
        m_ring[w].entityId = entityId;
        m_ring[w].text = std::move(text);
        m_ring[w].reasoning = std::move(reasoning);
        m_ring[w].success = success;
        m_ring[w].latencyMs = latencyMs;
        m_writeHead.store(next, std::memory_order_release);
        return true;
    }

    // Drain all pending results on the main thread
    // BUG FIX: Capture all fields BEFORE moving text to prevent UB from
    // accessing fields of a partially-moved-from struct
    template <typename ApplyFn>
    void Drain(ApplyFn&& apply) {
        uint32_t w = m_writeHead.load(std::memory_order_acquire);
        uint32_t r = m_readHead.load(std::memory_order_relaxed);
        while (r != w) {
            uint64_t eid = m_ring[r].entityId;
            bool ok = m_ring[r].success;
            std::string txt = std::move(m_ring[r].text);
            apply(eid, txt, ok);
            r = (r + 1) & MASK;
        }
        m_readHead.store(r, std::memory_order_release);
    }

    template <typename ApplyFn>
    void DrainEx(ApplyFn&& apply) {
        uint32_t w = m_writeHead.load(std::memory_order_acquire);
        uint32_t r = m_readHead.load(std::memory_order_relaxed);
        while (r != w) {
            uint64_t eid = m_ring[r].entityId;
            bool ok = m_ring[r].success;
            std::string reason = std::move(m_ring[r].reasoning);
            std::string txt = std::move(m_ring[r].text);
            apply(eid, txt, reason, ok);
            r = (r + 1) & MASK;
        }
        m_readHead.store(r, std::memory_order_release);
    }

private:
    alignas(64) SLMResult m_ring[CAPACITY];
    alignas(64) std::atomic<uint32_t> m_writeHead{0};
    alignas(64) std::atomic<uint32_t> m_readHead{0};
};

} // namespace slm
