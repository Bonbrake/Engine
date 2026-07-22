#pragma once
#include <string>
#include <functional>

namespace slm {

// Local LLM/SLM client (M11)
class SLMClient {
public:
    SLMClient();
    ~SLMClient();

    bool initialize(const std::string& endpoint, const std::string& apiKey = "");
    void shutdown();

    bool sendPrompt(const std::string& prompt, std::string& response);
    bool sendPromptStream(const std::string& prompt,
                          std::function<void(const std::string&)> onChunk);
    bool isAvailable() const;

private:
    std::string endpoint_;
    std::string apiKey_;
    bool initialized_ = false;
};

} // namespace slm
