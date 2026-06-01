#pragma once
#include <memory>
#include <string>

namespace slate::core {

// Application context - bukan singleton, dibikin eksplisit
class ApplicationContext {
public:
    ApplicationContext();
    ~ApplicationContext();

    void init_logging(const std::string& log_level = "info");
    void shutdown();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace slate::core
