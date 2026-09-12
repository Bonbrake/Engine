#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include "ze/slm/SPSCQueue.h"
#include "ze/slm/HermesPromptFormatter.h"
#include "ze/slm/HermesToolParser.h"
#include "ze/slm/HermesGrammar.h"
#include "ze/slm/HermesContext.h"

namespace slm {

enum class BackendType {
    EmbeddedLlamaGGUF,  // In-process GGUF via llama.cpp (MiniCPM5-1B / Qwen-2.5)
    OllamaLocal,        // REST API: http://localhost:11434/v1/chat/completions
    LMStudioLocal,      // REST API: http://localhost:1234/v1/chat/completions
    OpenRouterCloud     // Free Tier REST API: https://openrouter.ai/api/v1/chat/completions
};

struct SLMConfig {
    BackendType backend = BackendType::EmbeddedLlamaGGUF;
    std::string modelPath = "assets/models/Qwen2.5-3B-Instruct.Q4_K_M.gguf";
    std::string endpoint = "http://localhost:11434/v1/chat/completions";
    std::string apiKey = "";
    std::string modelName = "Qwen2.5-3B-Instruct";

    // Sampling parameters (optimized for deterministic in-game generation)
    uint32_t contextTokens = 2048;      // Ring-fenced KV cache context window (fits reasoning + generation)
    uint32_t maxOutputTokens = 512;     // Max tokens to generate per request (sufficient for CoT + output)
    float temperature = 0.7f;           // 0.0 = deterministic, 0.7 = creative
    float topP = 0.9f;                  // Nucleus sampling threshold
    float repeatPenalty = 1.1f;         // Penalize token repetition
    float presencePenalty = 0.1f;       // Encourage topic diversity

    // Production safety
    uint32_t requestTimeoutMs = 5000;   // Max ms before request is considered timed-out
    uint32_t maxRetries = 1;            // Retry count for failed inferences
};

struct SLMStats {
    std::atomic<uint64_t> totalRequests{0};
    std::atomic<uint64_t> totalCompleted{0};
    std::atomic<uint64_t> totalFailed{0};
    std::atomic<uint64_t> totalDropped{0};
    std::atomic<uint64_t> totalTimeoutsMs{0};
};

class SLMClient {
public:
    SLMClient();
    ~SLMClient();

    // Lifecycle
    bool initialize(const SLMConfig& config = SLMConfig());
    void shutdown();

    // Runtime backend switching (hot-swappable without restart)
    void setBackend(BackendType type, const std::string& endpoint = "", const std::string& apiKey = "");

    // Submit a request from the main render thread (lock-free, non-blocking)
    // mode == SLMMode::Fast: sub-300ms direct generation (bypasses <think>)
    // mode == SLMMode::Think: deep deliberative reasoning (generates full <think> CoT)
    bool submitRequest(uint64_t entityId, const std::string& tag, const std::string& prompt, SLMMode mode = SLMMode::Fast);

    // Drain completed results on the main render thread
    void updateMainThread(std::function<void(uint64_t entityId, const std::string& text, bool success)> onResult);
    void updateMainThreadEx(std::function<void(uint64_t entityId, const std::string& text, const std::string& reasoning, bool success)> onResult);

    // State queries
    bool isAvailable() const;
    BackendType getActiveBackend() const { return config_.backend; }
    const HermesContextResult& getDiscoveredContext() const { return contextResult_; }
    const SLMStats& getStats() const { return stats_; }

    // Build an OpenAI-compatible REST API payload (for Ollama, LM Studio, OpenRouter)
    static std::string buildOpenAIRestPayload(
        const std::string& modelName,
        const std::string& systemPrompt,
        const std::string& userPrompt,
        const SLMConfig& config = SLMConfig(),
        const std::string& responseFormat = ""
    );

private:
    void workerLoop();
    bool executeInference(const SLMRequest& req, std::string& outResponse, std::string& outReasoning);

    SLMConfig config_;
    HermesContextResult contextResult_;
    std::string systemPrompt_;
    SLMStats stats_;

    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::thread workerThread_;

    SPSCRequestQueue requestQueue_;
    SLMResultQueue resultQueue_;
};

} // namespace slm
