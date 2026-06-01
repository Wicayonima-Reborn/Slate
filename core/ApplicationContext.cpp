#include "ApplicationContext.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace slate::core {

struct ApplicationContext::Impl {
    bool initialized = false;
};

ApplicationContext::ApplicationContext()
    : pimpl_(std::make_unique<Impl>()) {}

ApplicationContext::~ApplicationContext() {
    shutdown();
}

void ApplicationContext::init_logging(const std::string& log_level) {
    if (pimpl_->initialized) return;
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("slate", std::move(console_sink));
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::from_str(log_level));
    pimpl_->initialized = true;
    spdlog::info("Slate core logging initialized");
}

void ApplicationContext::shutdown() {
    if (pimpl_->initialized) {
        spdlog::info("Slate core shutting down");
        spdlog::shutdown();
        pimpl_->initialized = false;
    }
}

} // namespace slate::core
