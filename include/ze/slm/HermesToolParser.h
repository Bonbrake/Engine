#pragma once

#include <string>
#include <vector>

namespace slm {

struct ParsedToolCall {
    std::string name;
    std::string argumentsJson;   // Just the "arguments" object, NOT the full blob
};

struct ParsedToolResponse {
    std::string name;
    std::string contentJson;     // The "content" field value
};

struct HermesParseResult {
    // Internal reasoning (stripped from player-facing output)
    std::string thought;                        // <thought>...</thought>
    std::string scratchpad;                     // <scratchpad>...</scratchpad>  (Hermes 3 agentic)
    std::string reasoning;                      // <reasoning>...</reasoning>    (Hermes 3 agentic)
    std::string innerMonologue;                 // <inner_monologue>...</inner_monologue>
    std::string plan;                           // <plan>...</plan>
    std::string think;                          // <think>...</think>  (DeepHermes 3)

    std::string rawProse;                       // Remaining text after all tags removed
    std::vector<ParsedToolCall> toolCalls;
    std::vector<ParsedToolResponse> toolResponses;

    bool hasToolCall = false;
    bool hasToolResponse = false;
    bool hasThought = false;
    bool hasReasoning = false;                  // Any internal reasoning tag was present
};

class HermesToolParser {
public:
    // Parse model output extracting all Hermes XML tags and JSON payloads
    static HermesParseResult parseOutput(const std::string& modelOutput);

    // Format a tool response for feeding back into conversation
    static std::string formatToolResponse(const std::string& toolName, const std::string& responseJson);

    // Validate that a JSON string conforms to an expected set of required keys
    static bool validateJsonKeys(const std::string& jsonStr, const std::vector<std::string>& requiredKeys);
};

} // namespace slm
