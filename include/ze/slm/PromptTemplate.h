#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace slm {

// Template renderer for LLM prompts (M11)
class PromptTemplate {
public:
    // Replace {{variable}} placeholders with values
    static std::string render(const std::string& templateStr,
                              const std::unordered_map<std::string, std::string>& vars);

    // Extract all {{variable}} names from template
    static std::vector<std::string> extractVariables(const std::string& templateStr);
};

} // namespace slm
