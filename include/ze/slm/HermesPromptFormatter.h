#pragma once

#include "ze/slm/SPSCQueue.h"
#include <string>
#include <vector>

namespace slm {

struct HermesMessage {
    std::string role; // "system", "user", "assistant", "tool"
    std::string content;
    std::string name; // Optional name tag (e.g. tool name for role="tool")
};

struct HermesToolParam {
    std::string name;
    std::string type; // "string", "number", "integer", "boolean", "object", "array"
    std::string description;
    bool required = false;
};

struct HermesToolDef {
    std::string name;
    std::string description;
    std::vector<HermesToolParam> parameters;
};

class HermesPromptFormatter {
public:
    // Format a full ChatML prompt with system, tools, history, and trailing assistant turn.
    // When mode == SLMMode::Fast, emits direct <|im_start|>assistant\n for immediate sub-250ms JSON generation.
    // When mode == SLMMode::Think, opens <thought>\n for deliberate tactical CoT exploration.
    static std::string formatChatML(
        const std::string& systemPrompt,
        const std::vector<HermesMessage>& conversationHistory,
        const std::vector<HermesToolDef>& tools = {},
        SLMMode mode = SLMMode::Fast
    );

    // Build the <tools>...</tools> block using the OFFICIAL Hermes 3 schema format:
    //   {"type": "function", "function": {"name": ..., "description": ..., "parameters": {...}}}
    static std::string buildToolsBlock(const std::vector<HermesToolDef>& tools);

    // Format a tool_call message for multi-turn conversations
    static std::string formatToolCallMessage(const std::string& toolName, const std::string& argsJson);

    // Format a tool_response turn for feeding results back to the model
    static std::string formatToolResponseTurn(const std::string& toolName, const std::string& responseJson);
};

} // namespace slm
