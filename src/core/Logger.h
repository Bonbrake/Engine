#pragma once

#include <spdlog/spdlog.h>
#include <memory>

#define LOG_TRACE(...)    ::core::Logger::get()->trace(__VA_ARGS__)
#define LOG_INFO(...)     ::core::Logger::get()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::core::Logger::get()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::core::Logger::get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::core::Logger::get()->critical(__VA_ARGS__)

namespace core {

class Logger {
public:
    static void init();
    static std::shared_ptr<spdlog::logger>& get() { return logger_; }

private:
    static std::shared_ptr<spdlog::logger> logger_;
};

} // namespace core
