#include "opengame_core.hpp"

namespace opengame {

bool Core::initialize() noexcept {
    if (state_ != SessionState::stopped) {
        return false;
    }

    state_ = SessionState::initialized;
    return true;
}

bool Core::start(EngineKind /*engine*/) noexcept {
    if (state_ != SessionState::initialized) {
        return false;
    }

    // Engine execution is not implemented in foundation release 0.1.
    state_ = SessionState::running;
    return true;
}

void Core::stop() noexcept {
    state_ = SessionState::stopped;
}

SessionState Core::state() const noexcept {
    return state_;
}

std::string_view Core::version() const noexcept {
    return "0.1.0";
}

} // namespace opengame
