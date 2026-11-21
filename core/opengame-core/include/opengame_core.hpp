#pragma once

#include <cstdint>
#include <string_view>

namespace opengame {

enum class EngineKind {
    pc,
    ps3
};

enum class CpuArchitecture {
    unknown,
    arm32,
    arm64,
    x86,
    x64
};

enum class SessionState {
    stopped,
    initialized,
    running,
    error
};

enum class LogLevel {
    debug,
    info,
    warning,
    error
};

struct DeviceProfile {
    CpuArchitecture cpu_architecture{CpuArchitecture::unknown};
    std::uint32_t cpu_cores{0};
    std::uint64_t total_memory_bytes{0};
    std::uint32_t android_api_level{0};
    bool vulkan_supported{false};
    std::uint8_t opengl_es_major{0};
    std::uint8_t opengl_es_minor{0};
};

struct SessionConfig {
    EngineKind engine{EngineKind::pc};
    std::string_view content_directory{};
    std::uint32_t render_width{1280};
    std::uint32_t render_height{720};
    bool prefer_vulkan{true};
};

using LogCallback = void (*)(LogLevel level,
                             std::string_view message,
                             void* user_data) noexcept;

struct Logger {
    LogCallback callback{nullptr};
    void* user_data{nullptr};
};

class Core final {
public:
    Core() = default;

    [[nodiscard]] bool initialize(const DeviceProfile& device,
                                  const SessionConfig& config,
                                  Logger logger = {}) noexcept;
    [[nodiscard]] bool start() noexcept;
    void stop() noexcept;

    [[nodiscard]] SessionState state() const noexcept;
    [[nodiscard]] std::string_view version() const noexcept;
    [[nodiscard]] const DeviceProfile& device_profile() const noexcept;
    [[nodiscard]] const SessionConfig& session_config() const noexcept;

private:
    void log(LogLevel level, std::string_view message) const noexcept;

    SessionState state_{SessionState::stopped};
    DeviceProfile device_{};
    SessionConfig config_{};
    Logger logger_{};
};

} // namespace opengame
