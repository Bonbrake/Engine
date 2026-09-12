#pragma once

#include <string>
#include <vector>

namespace slm {

struct HermesContextResult {
    std::string rootHermesDoc;  // Content of .hermes.md / HERMES.md found walking up to git root
    std::string agentsDoc;      // Content of AGENTS.md found in CWD (truncated if > 8k)
    std::string soulDoc;        // Content of SOUL.md from global config
    std::string userMemoryDoc;  // Content of MEMORY.md / USER.md
    bool foundHermesRoot = false;
    bool foundAgentsDoc = false;
};

class HermesContext {
public:
    static HermesContextResult discoverContext(const std::string& startPath = ".");
    static std::string buildContextPrompt(const HermesContextResult& context);
};

} // namespace slm
