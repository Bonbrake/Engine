#include "ze/slm/PromptTemplate.h"
#include <sstream>
#include <algorithm>

namespace slm {

std::string PromptTemplate::render(const std::string& templateStr,
                                    const std::unordered_map<std::string, std::string>& vars) {
    std::string result = templateStr;
    for (const auto& [key, value] : vars) {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    return result;
}

std::vector<std::string> PromptTemplate::extractVariables(const std::string& templateStr) {
    std::vector<std::string> vars;
    size_t pos = 0;
    while ((pos = templateStr.find("{{", pos)) != std::string::npos) {
        size_t end = templateStr.find("}}", pos);
        if (end == std::string::npos) break;
        std::string varName = templateStr.substr(pos + 2, end - pos - 2);
        vars.push_back(varName);
        pos = end + 2;
    }
    return vars;
}

} // namespace slm
