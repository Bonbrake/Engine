#include "ze/slm/HermesContext.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace slm {

static constexpr size_t MAX_FILE_READ_BYTES = 64 * 1024; // 64KB hard cap on any context file

static std::string readFileContent(const fs::path& filePath, size_t maxChars = 0) {
    std::error_code ec;
    if (!fs::exists(filePath, ec) || !fs::is_regular_file(filePath, ec)) return "";

    // Guard against reading enormous files into memory
    auto fileSize = fs::file_size(filePath, ec);
    if (ec || fileSize == 0) return "";
    if (fileSize > MAX_FILE_READ_BYTES) fileSize = MAX_FILE_READ_BYTES;

    std::ifstream f(filePath, std::ios::in | std::ios::binary);
    if (!f.is_open()) return "";

    std::string content;
    content.resize(static_cast<size_t>(fileSize));
    f.read(content.data(), static_cast<std::streamsize>(fileSize));
    content.resize(static_cast<size_t>(f.gcount()));

    if (maxChars > 0 && content.size() > maxChars) {
        content = content.substr(0, maxChars) + "\n\n[... truncated at " + std::to_string(maxChars) + " char cap ...]";
    }
    return content;
}

HermesContextResult HermesContext::discoverContext(const std::string& startPath) {
    HermesContextResult res;
    std::error_code ec;
    fs::path curr = fs::absolute(startPath, ec);
    if (ec) return res;

    // 1. Walk up looking for .hermes.md or HERMES.md (highest priority)
    //    Check for the file FIRST at each level, THEN check stop conditions.
    //    This ensures we find .hermes.md even if it lives alongside .git
    fs::path p = curr;
    constexpr int MAX_WALK_DEPTH = 32; // Safety limit against pathological symlink loops
    for (int depth = 0; depth < MAX_WALK_DEPTH; ++depth) {
        fs::path h1 = p / ".hermes.md";
        fs::path h2 = p / "HERMES.md";
        if (fs::exists(h1, ec)) {
            res.rootHermesDoc = readFileContent(h1);
            res.foundHermesRoot = true;
            break;
        } else if (fs::exists(h2, ec)) {
            res.rootHermesDoc = readFileContent(h2);
            res.foundHermesRoot = true;
            break;
        }

        // Stop conditions AFTER checking for hermes files at this level
        if (fs::exists(p / ".git", ec)) {
            break; // Reached git root without finding hermes file
        }
        fs::path parent = p.parent_path();
        if (parent == p || !p.has_parent_path()) {
            break; // Reached filesystem root
        }
        p = parent;
    }

    // 2. Discover AGENTS.md in the start directory (capped at 8,000 chars per spec)
    //    Fall back to CLAUDE.md or .cursorrules if AGENTS.md does not exist
    const char* agentFileNames[] = { "AGENTS.md", "CLAUDE.md", ".cursorrules" };
    for (const char* name : agentFileNames) {
        fs::path agentPath = curr / name;
        if (fs::exists(agentPath, ec)) {
            res.agentsDoc = readFileContent(agentPath, 8000);
            res.foundAgentsDoc = true;
            break;
        }
    }

    // 3. Discover optional SOUL.md and MEMORY.md/USER.md from git root or start dir
    fs::path soulPath = curr / "SOUL.md";
    if (fs::exists(soulPath, ec)) {
        res.soulDoc = readFileContent(soulPath, 4000);
    }

    const char* memoryFileNames[] = { "MEMORY.md", "USER.md" };
    for (const char* name : memoryFileNames) {
        fs::path memPath = curr / name;
        if (fs::exists(memPath, ec)) {
            res.userMemoryDoc = readFileContent(memPath, 4000);
            break;
        }
    }

    return res;
}

std::string HermesContext::buildContextPrompt(const HermesContextResult& context) {
    std::ostringstream ss;
    if (context.foundHermesRoot && !context.rootHermesDoc.empty()) {
        ss << "=== PROJECT CONSTITUTION (.hermes.md) ===\n";
        ss << context.rootHermesDoc << "\n\n";
    }
    if (context.foundAgentsDoc && !context.agentsDoc.empty()) {
        ss << "=== LOCAL AGENT CONVENTIONS (AGENTS.md) ===\n";
        ss << context.agentsDoc << "\n\n";
    }
    if (!context.soulDoc.empty()) {
        ss << "=== AGENT PERSONA (SOUL.md) ===\n";
        ss << context.soulDoc << "\n\n";
    }
    if (!context.userMemoryDoc.empty()) {
        ss << "=== USER MEMORY ===\n";
        ss << context.userMemoryDoc << "\n\n";
    }
    return ss.str();
}

} // namespace slm
