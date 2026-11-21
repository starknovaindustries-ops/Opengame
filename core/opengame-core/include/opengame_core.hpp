#pragma once

#include <string_view>

namespace opengame {

enum class EngineKind {
    pc,
    ps3
};

enum class SessionState {
    stopped,
    initialized,
    running,
    error
};

class Core final {
public:
    Core() = default;

    [[nodiscard]] bool initialize() noexcept;
    [[nodiscard]] bool start(EngineKind engine) noexcept;
    void stop() noexcept;

    [[nodiscard]] SessionState state() const noexcept;
    [[nodiscard]] std::string_view version() const noexcept;

private:
    SessionState state_{SessionState::stopped};
};

} // namespace opengame
