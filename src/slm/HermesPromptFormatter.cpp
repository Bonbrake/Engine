#include "ze/slm/HermesPromptFormatter.h"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace slm {

// Strip ChatML special tokens to prevent prompt injection attacks
// where user-supplied text contains <|im_start|> or <|im_end|> to
// hijack the conversation structure
static std::string sanitizeChatML(const std::string& input) {
    std::string text = input;
    const char* injectionTokens[] = { "<|im_start|>", "<|im_end|>" };
    for (const char* token : injectionTokens) {
        std::string tok(token);
        size_t pos = 0;
        while ((pos = text.find(tok, pos)) != std::string::npos) {
            text.erase(pos, tok.length());
        }
    }
    return text;
}

std::string HermesPromptFormatter::buildToolsBlock(const std::vector<HermesToolDef>& tools) {
    if (tools.empty()) return "";

    // OFFICIAL Hermes 3 tool definition format (per NousResearch docs):
    //   {"type": "function", "function": {"name": ..., "description": ..., "parameters": {...}}}
    // Each tool is a separate JSON object on its own line inside <tools>...</tools>
    std::ostringstream ss;
    ss << "<tools>\n";

    for (const auto& t : tools) {
        json funcObj;
        funcObj["name"] = t.name;
        funcObj["description"] = t.description;

        json propsObj = json::object();
        json reqArr = json::array();

        for (const auto& p : t.parameters) {
            json pObj;
            pObj["type"] = p.type.empty() ? "string" : p.type;
            pObj["description"] = p.description;
            propsObj[p.name] = pObj;

            if (p.required) {
                reqArr.push_back(p.name);
            }
        }

        json paramsObj;
        paramsObj["type"] = "object";
        paramsObj["properties"] = propsObj;
        paramsObj["required"] = reqArr;

        funcObj["parameters"] = paramsObj;

        // Wrap in the official {"type": "function", "function": {...}} envelope
        json toolEnvelope;
        toolEnvelope["type"] = "function";
        toolEnvelope["function"] = funcObj;

        ss << toolEnvelope.dump() << "\n";
    }

    ss << "</tools>";
    return ss.str();
}

std::string HermesPromptFormatter::formatChatML(
    const std::string& systemPrompt,
    const std::vector<HermesMessage>& conversationHistory,
    const std::vector<HermesToolDef>& tools,
    SLMMode mode
) {
    std::ostringstream ss;

    // System message block with official Hermes function calling preamble
    ss << "<|im_start|>system\n";

    if (!tools.empty()) {
        // Official Hermes 3 system prompt preamble for function calling
        ss << "You are a function calling AI model. You are provided with function "
              "signatures within <tools></tools> XML tags. You may call one or more "
              "functions to assist with the user query. Don't make assumptions about "
              "what values to plug into functions. Here are the available tools:\n";
        ss << buildToolsBlock(tools) << "\n";

        // Append custom system context after the tools preamble
        if (!systemPrompt.empty()) {
            ss << "\n" << sanitizeChatML(systemPrompt);
        }
    } else {
        ss << sanitizeChatML(systemPrompt);
    }
    ss << "<|im_end|>\n";

    // Conversation turns
    for (const auto& msg : conversationHistory) {
        ss << "<|im_start|>" << sanitizeChatML(msg.role);
        if (!msg.name.empty()) {
            ss << " name=" << sanitizeChatML(msg.name);
        }
        ss << "\n" << sanitizeChatML(msg.content) << "<|im_end|>\n";
    }

    // Trailing assistant turn to prompt generation
    // Dual-mode reasoning control:
    // Fast mode: prefill empty <think>\n</think>\n to skip reasoning entirely (sub-300ms)
    // Think mode: opens <think>\n to trigger deep chain-of-thought deliberation
    ss << "<|im_start|>assistant\n";
    if (mode == SLMMode::Fast) {
        ss << "<think>\n</think>\n";
    } else if (mode == SLMMode::Think) {
        ss << "<think>\n";
    }
    return ss.str();
}

std::string HermesPromptFormatter::formatToolCallMessage(
    const std::string& toolName,
    const std::string& argsJson
) {
    // Format as the assistant's tool invocation turn
    json callObj;
    callObj["name"] = toolName;

    auto argsParsed = json::parse(argsJson, nullptr, false);
    if (!argsParsed.is_discarded()) {
        callObj["arguments"] = argsParsed;
    } else {
        callObj["arguments"] = json::object();
    }

    std::ostringstream ss;
    ss << "<|im_start|>assistant\n";
    ss << "<tool_call>\n";
    ss << callObj.dump() << "\n";
    ss << "</tool_call><|im_end|>\n";
    return ss.str();
}

std::string HermesPromptFormatter::formatToolResponseTurn(
    const std::string& toolName,
    const std::string& responseJson
) {
    // Format as the tool's response turn back to the model
    std::ostringstream ss;
    ss << "<|im_start|>tool name=" << toolName << "\n";
    ss << "<tool_response>\n";
    ss << responseJson << "\n";
    ss << "</tool_response><|im_end|>\n";
    return ss.str();
}

} // namespace slm
