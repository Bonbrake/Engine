#include "ze/slm/SLMClient.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;

namespace slm {

SLMClient::SLMClient() = default;

SLMClient::~SLMClient() {
    shutdown();
}

bool SLMClient::initialize(const SLMConfig& config) {
    if (initialized_) return true;

    config_ = config;

    // Discover repository context metadata if present (for telemetry / developer tools)
    contextResult_ = HermesContext::discoverContext(".");

    // Ensure system prompt is focused and concise (~60 tokens) for real-time in-game AI Director.
    // Avoid dumping thousands of tokens of repository markdown into the runtime inference context.
    systemPrompt_ = "You are the ZombieEngine AI Director, powered by Qwen2.5-3B-Instruct. "
                    "Generate structured JSON outputs conforming strictly to the requested schema. "
                    "Be concise, deterministic, and maintain high survival tension. Do not hallucinate data.";

    running_ = true;
    
    // Launch background worker thread for non-blocking token generation
    workerThread_ = std::thread(&SLMClient::workerLoop, this);
    initialized_ = true;

    spdlog::info("SLMClient initialized | backend={} model={} ctx={} timeout={}ms hermes_root={}",
        static_cast<int>(config_.backend), config_.modelName,
        config_.contextTokens, config_.requestTimeoutMs,
        contextResult_.foundHermesRoot);
    return true;
}

void SLMClient::shutdown() {
    if (!initialized_) return;

    running_ = false;
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
    initialized_ = false;

    spdlog::info("SLMClient shutdown | total={} completed={} failed={} dropped={}",
        stats_.totalRequests.load(), stats_.totalCompleted.load(),
        stats_.totalFailed.load(), stats_.totalDropped.load());
}

void SLMClient::setBackend(BackendType type, const std::string& endpoint, const std::string& apiKey) {
    config_.backend = type;
    if (!endpoint.empty()) config_.endpoint = endpoint;
    if (!apiKey.empty()) config_.apiKey = apiKey;
    spdlog::info("SLMClient backend hot-swapped to {} endpoint={}", static_cast<int>(type), config_.endpoint);
}

bool SLMClient::submitRequest(uint64_t entityId, const std::string& tag, const std::string& prompt, SLMMode mode) {
    if (!initialized_) return false;

    stats_.totalRequests++;
    bool pushed = requestQueue_.Push(entityId, tag, prompt, mode);
    if (!pushed) {
        stats_.totalDropped++;
        spdlog::warn("SLMClient request queue full, dropping tag={} entity={}", tag, entityId);
    }
    return pushed;
}

void SLMClient::updateMainThread(std::function<void(uint64_t entityId, const std::string& text, bool success)> onResult) {
    if (!initialized_) return;
    resultQueue_.Drain(onResult);
}

void SLMClient::updateMainThreadEx(std::function<void(uint64_t entityId, const std::string& text, const std::string& reasoning, bool success)> onResult) {
    if (!initialized_) return;
    resultQueue_.DrainEx(onResult);
}

bool SLMClient::isAvailable() const {
    return initialized_.load();
}

std::string SLMClient::buildOpenAIRestPayload(
    const std::string& modelName,
    const std::string& systemPrompt,
    const std::string& userPrompt,
    const SLMConfig& config,
    const std::string& responseFormat
) {
    json body;
    body["model"] = modelName;
    body["temperature"] = config.temperature;
    body["top_p"] = config.topP;
    body["max_tokens"] = config.maxOutputTokens;
    body["stream"] = false;

    // Repetition/presence penalty (supported by Ollama, LM Studio, OpenRouter)
    if (config.repeatPenalty != 1.0f) {
        body["repeat_penalty"] = config.repeatPenalty;
    }
    if (config.presencePenalty != 0.0f) {
        body["presence_penalty"] = config.presencePenalty;
    }

    json messages = json::array();
    messages.push_back({ { "role", "system" }, { "content", systemPrompt } });
    messages.push_back({ { "role", "user" }, { "content", userPrompt } });
    body["messages"] = messages;

    // Structured output (JSON schema) for servers that support response_format
    if (!responseFormat.empty()) {
        auto fmt = json::parse(responseFormat, nullptr, false);
        if (!fmt.is_discarded()) {
            body["response_format"] = fmt;
        }
    }

    return body.dump(2);
}

void SLMClient::workerLoop() {
    while (running_) {
        SLMRequest req;
        if (requestQueue_.Pop(req)) {
            auto startTime = std::chrono::steady_clock::now();

            // Check if request has already timed out waiting in queue
            auto queueWaitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                startTime - req.submitTime).count();
            if (config_.requestTimeoutMs > 0 &&
                static_cast<uint32_t>(queueWaitMs) > config_.requestTimeoutMs) {
                spdlog::warn("SLMClient request timed out in queue ({}ms > {}ms) tag={} entity={}",
                    queueWaitMs, config_.requestTimeoutMs, req.tag, req.entityId);
                stats_.totalFailed++;
                // Push a failure result so the ECS system can handle gracefully
                resultQueue_.Push(req.entityId, "{\"error\":\"timeout\"}", false, static_cast<float>(queueWaitMs));
                continue;
            }

            std::string responseText;
            std::string reasoningText;
            bool ok = false;

            // Retry loop
            for (uint32_t attempt = 0; attempt <= config_.maxRetries; ++attempt) {
                ok = executeInference(req, responseText, reasoningText);
                if (ok) break;

                if (attempt < config_.maxRetries) {
                    spdlog::warn("SLMClient inference failed, retrying (attempt {}/{}) tag={}",
                        attempt + 1, config_.maxRetries, req.tag);
                }
            }

            auto endTime = std::chrono::steady_clock::now();
            float latencyMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

            if (ok) {
                stats_.totalCompleted++;
            } else {
                stats_.totalFailed++;
            }

            bool pushed = resultQueue_.Push(req.entityId, responseText, ok, latencyMs, reasoningText);
            if (!pushed) {
                stats_.totalDropped++;
                spdlog::warn("SLMClient result queue full, dropping completed result entity={}", req.entityId);
            }
        } else {
            // No work available - yield CPU time
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
}

bool SLMClient::executeInference(const SLMRequest& req, std::string& outResponse, std::string& outReasoning) {
    // 1. Select schema & GBNF grammar based on M13 request tag
    JSONSchema schema;
    bool useSchema = false;

    if (req.tag == "M13-EXT-25" || req.tag == "DIALOGUE") {
        schema = HermesGrammar::getDialogueSchema();
        useSchema = true;
    } else if (req.tag == "M13-EXT-26" || req.tag == "LORE") {
        schema = HermesGrammar::getLoreNoteSchema();
        useSchema = true;
    } else if (req.tag == "M13-EXT-15" || req.tag == "RADIO") {
        schema = HermesGrammar::getRadioBroadcastSchema();
        useSchema = true;
    } else if (req.tag == "M13-EXT-27" || req.tag == "BUILD_ADVISOR") {
        schema = HermesGrammar::getBuildAdviceSchema();
        useSchema = true;
    }

    // 2. Build the formatted prompt
    std::string userPrompt = req.promptFields.empty() ? "Generate content" : req.promptFields;
    std::vector<HermesMessage> history = { { "user", userPrompt, "" } };

    // Compile GBNF grammar for constrained sampling (embedded backend)
    std::string gbnfGrammar;
    if (useSchema && config_.backend == BackendType::EmbeddedLlamaGGUF) {
        gbnfGrammar = HermesGrammar::compileToGBNF(schema);
    }

    // Build ChatML prompt (used by embedded backend) with dual-mode Fast vs Think routing
    std::string formattedPrompt = HermesPromptFormatter::formatChatML(systemPrompt_, history, {}, req.mode);

    // 3. Build REST payload for remote backends
    if (config_.backend != BackendType::EmbeddedLlamaGGUF) {
        std::string responseFormat;
        if (useSchema) {
            responseFormat = HermesGrammar::toJsonSchemaString(schema);
        }

        std::string restPayload = buildOpenAIRestPayload(
            config_.modelName, systemPrompt_, userPrompt, config_, responseFormat);

        spdlog::debug("SLMClient REST payload ready | endpoint={} model={} schema={}",
            config_.endpoint, config_.modelName, useSchema ? schema.title : "none");

        // TODO: HTTP POST via ze-net curl/WinHTTP integration
        // For now, fall through to stub generation below
    }

    // 4. Generate structured schema response (stub until llama.cpp/HTTP integration)
    if (req.mode == SLMMode::Think) {
        outReasoning = "Analyzing tactical situation and player threat level; selecting optimal narrative response.";
    } else {
        outReasoning.clear();
    }

    if (useSchema) {
        json jObj;
        if (schema.title == "NPCDialogue") {
            jObj["speaker"] = "Survivor";
            jObj["emotion"] = "neutral";
            jObj["text"] = userPrompt;
        } else if (schema.title == "RadioBroadcast") {
            jObj["frequency"] = 104.5;
            jObj["callsign"] = "Station-Alpha";
            jObj["transmission"] = userPrompt;
        } else if (schema.title == "EnvironmentalLore") {
            jObj["author"] = "Dr. Miller";
            jObj["category"] = "journal";
            jObj["content"] = userPrompt;
        } else if (schema.title == "BuildAdvisor") {
            jObj["playstyle"] = "stealth";
            jObj["recommendedPerk"] = "Silent Step";
            jObj["rationale"] = userPrompt;
        }
        outResponse = jObj.dump();
    } else {
        json jObj;
        jObj["tag"] = req.tag;
        jObj["entityId"] = req.entityId;
        jObj["generated_content"] = userPrompt;
        outResponse = jObj.dump();
    }

    // 5. Validate output structure
    if (useSchema) {
        bool valid = HermesToolParser::validateJsonKeys(outResponse, schema.required);
        if (!valid) {
            spdlog::error("SLMClient output failed schema validation for {}", schema.title);
            return false;
        }
    }

    return true;
}

} // namespace slm
