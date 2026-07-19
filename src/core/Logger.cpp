#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <vector>

namespace core {

std::shared_ptr<spdlog::logger> Logger::logger_;

void Logger::init() {
    if (logger_) return;  // idempotent: spdlog::register_logger throws on duplicate name "ENGINE"
    spdlog::init_thread_pool(8192, 1);

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // standard format
    stdout_sink->set_pattern("%^[%T] %n: %v%$");

    // JSON-lines structured log sink (logs/session.jsonl)
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/session.jsonl", true);
    // basic format for jsonl; in a real scenario we'd use a custom json formatter
    file_sink->set_pattern("{\"timestamp\":\"%Y-%m-%dT%H:%M:%S.%f\",\"level\":\"%l\",\"message\":\"%v\"}");

    std::vector<spdlog::sink_ptr> sinks {stdout_sink, file_sink};

    logger_ = std::make_shared<spdlog::async_logger>("ENGINE", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::register_logger(logger_);
    logger_->set_level(spdlog::level::trace);
    logger_->flush_on(spdlog::level::trace);
}

} // namespace core
