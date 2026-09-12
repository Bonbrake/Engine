#include "ze/slm/HermesToolParser.h"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace slm {

// Extract all instances of <tagName>...</tagName> from text, appending content
// to `out`. Handles truncated (unclosed) tags gracefully. Returns modified text
// with extracted blocks removed.
static std::string extractXmlBlocks(
    const std::string& text,
    const std::string& tagName,
    std::string& out,
    bool& found
) {
    std::string result = text;
    std::string openTag = "<" + tagName + ">";
    std::string closeTag = "</" + tagName + ">";

    size_t pos = 0;
    while ((pos = result.find(openTag, pos)) != std::string::npos) {
        size_t contentStart = pos + openTag.length();
        size_t closePos = result.find(closeTag, contentStart);

        if (closePos != std::string::npos) {
            std::string content = result.substr(contentStart, closePos - contentStart);
            if (!out.empty()) out += "\n";
            out += content;
            found = true;
            result.erase(pos, (closePos + closeTag.length()) - pos);
        } else {
            // Unclosed tag (truncated generation) - extract remaining content
            // and stop so it doesn't leak into player-facing prose
            std::string content = result.substr(contentStart);
            if (!out.empty()) out += "\n";
            out += content;
            found = true;
            result.erase(pos);
            break;
        }
    }
    return result;
}

HermesParseResult HermesToolParser::parseOutput(const std::string& modelOutput) {
    HermesParseResult result;
    std::string text = modelOutput;

    // 1. Extract ALL official Hermes 3 internal reasoning tags
    //    These are never shown to players - only used for telemetry/debugging
    bool foundThought = false, foundScratch = false, foundReason = false;
    bool foundInner = false, foundPlan = false, foundThink = false;

    text = extractXmlBlocks(text, "thought", result.thought, foundThought);
    text = extractXmlBlocks(text, "scratchpad", result.scratchpad, foundScratch);
    text = extractXmlBlocks(text, "reasoning", result.reasoning, foundReason);
    text = extractXmlBlocks(text, "inner_monologue", result.innerMonologue, foundInner);
    text = extractXmlBlocks(text, "plan", result.plan, foundPlan);
    text = extractXmlBlocks(text, "think", result.think, foundThink);

    result.hasThought = foundThought;
    result.hasReasoning = foundScratch || foundReason || foundInner || foundPlan || foundThink;

    // 2. Extract <tool_call>...</tool_call> blocks
    size_t callPos = 0;
    while ((callPos = text.find("<tool_call>", callPos)) != std::string::npos) {
        size_t callEnd = text.find("</tool_call>", callPos);
        if (callEnd == std::string::npos) {
            // Truncated tool call - discard incomplete invocation
            text.erase(callPos);
            break;
        }

        std::string jsonPayload = text.substr(callPos + 11, callEnd - (callPos + 11));
        
        // Trim whitespace
        size_t firstNonSpace = jsonPayload.find_first_not_of(" \t\n\r");
        size_t lastNonSpace = jsonPayload.find_last_not_of(" \t\n\r");
        if (firstNonSpace != std::string::npos && lastNonSpace != std::string::npos) {
            jsonPayload = jsonPayload.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
        }

        ParsedToolCall tc;

        auto parsed = json::parse(jsonPayload, nullptr, false);
        if (!parsed.is_discarded() && parsed.is_object()) {
            // Extract "name" field
            if (parsed.contains("name") && parsed["name"].is_string()) {
                tc.name = parsed["name"].get<std::string>();
            }
            // Extract "arguments" as its own JSON string (official Hermes format)
            // NOT the entire blob - just the arguments object
            if (parsed.contains("arguments") && parsed["arguments"].is_object()) {
                tc.argumentsJson = parsed["arguments"].dump();
            } else {
                // Fallback: store full payload for non-conformant models
                tc.argumentsJson = jsonPayload;
            }
        } else {
            tc.argumentsJson = jsonPayload;
        }

        result.toolCalls.push_back(tc);
        result.hasToolCall = true;

        text.erase(callPos, (callEnd + 12) - callPos);
    }

    // 3. Extract <tool_response>...</tool_response> blocks
    size_t respPos = 0;
    while ((respPos = text.find("<tool_response>", respPos)) != std::string::npos) {
        size_t respEnd = text.find("</tool_response>", respPos);
        if (respEnd == std::string::npos) {
            text.erase(respPos);
            break;
        }

        std::string jsonPayload = text.substr(respPos + 15, respEnd - (respPos + 15));
        
        size_t firstNonSpace = jsonPayload.find_first_not_of(" \t\n\r");
        size_t lastNonSpace = jsonPayload.find_last_not_of(" \t\n\r");
        if (firstNonSpace != std::string::npos && lastNonSpace != std::string::npos) {
            jsonPayload = jsonPayload.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
        }

        ParsedToolResponse tr;
        tr.contentJson = jsonPayload;

        auto parsed = json::parse(jsonPayload, nullptr, false);
        if (!parsed.is_discarded() && parsed.is_object()) {
            if (parsed.contains("name") && parsed["name"].is_string()) {
                tr.name = parsed["name"].get<std::string>();
            }
            // Extract just the "content" field if present (official format)
            if (parsed.contains("content")) {
                tr.contentJson = parsed["content"].dump();
            }
        }

        result.toolResponses.push_back(tr);
        result.hasToolResponse = true;

        text.erase(respPos, (respEnd + 16) - respPos);
    }

    // 4. Trim remaining raw prose (player-visible text)
    size_t start = text.find_first_not_of(" \t\n\r");
    size_t end = text.find_last_not_of(" \t\n\r");
    if (start != std::string::npos && end != std::string::npos) {
        result.rawProse = text.substr(start, end - start + 1);
    } else {
        result.rawProse = "";
    }

    return result;
}

std::string HermesToolParser::formatToolResponse(const std::string& toolName, const std::string& responseJson) {
    json resObj;
    resObj["name"] = toolName;
    auto contentParsed = json::parse(responseJson, nullptr, false);
    if (!contentParsed.is_discarded()) {
        resObj["content"] = contentParsed;
    } else {
        resObj["content"] = responseJson;
    }

    std::ostringstream ss;
    ss << "<tool_response>\n";
    ss << resObj.dump() << "\n";
    ss << "</tool_response>";
    return ss.str();
}

bool HermesToolParser::validateJsonKeys(const std::string& jsonStr, const std::vector<std::string>& requiredKeys) {
    auto parsed = json::parse(jsonStr, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_object()) {
        return false;
    }
    for (const auto& key : requiredKeys) {
        if (!parsed.contains(key)) {
            return false;
        }
    }
    return true;
}

} // namespace slm
