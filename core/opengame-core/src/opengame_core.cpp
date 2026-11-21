#include "opengame_core.hpp"

namespace opengame {
namespace {

bool is_valid_engine(EngineKind engine) noexcept {
    switch (engine) {
        case EngineKind::pc:
        case EngineKind::ps3:
            return true;
    }
    return false;
}

} // namespace

bool Core::initialize(const DeviceProfile& device,
                      const SessionConfig& config,
                      Logger logger) noexcept {
    if (state_ != SessionState::stopped) {
        return false;
    }

    logger_ = logger;

    if (device.cpu_cores == 0 ||
        config.render_width == 0 ||
        config.render_height == 0 ||
        !is_valid_engine(config.engine)) {
        log(LogLevel::error, "Invalid device profile or session configuration.");
        return false;
    }

    device_ = device;
    config_ = config;
    state_ = SessionState::initialized;
    log(LogLevel::info, "Opengame core initialized; no game engine is loaded.");
    return true;
}

bool Core::start() noexcept {
    if (state_ != SessionState::initialized) {
        log(LogLevel::warning, "Session start requested before initialization.");
        return false;
    }

    // PC and PS3 execution backends are not implemented yet.
    log(LogLevel::warning, "Selected engine is not implemented; session was not started.");
    return false;
}

void Core::stop() noexcept {
    if (state_ != SessionState::stopped) {
        state_ = SessionState::stopped;
        log(LogLevel::info, "Opengame session stopped.");
    }
}

SessionState Core::state() const noexcept {
    return state_;
}

std::string_view Core::version() const noexcept {
    return "0.1.1";
}

const DeviceProfile& Core::device_profile() const noexcept {
    return device_;
}

const SessionConfig& Core::session_config() const noexcept {
    return config_;
}

void Core::log(LogLevel level, std::string_view message) const noexcept {
    if (logger_.callback != nullptr) {
        logger_.callback(level, message, logger_.user_data);
    }
}

} // namespace opengame
