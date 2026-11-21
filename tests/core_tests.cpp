#include "opengame_core.hpp"

#include <cassert>
#include <string_view>

namespace {

struct LogCapture {
    std::uint32_t entries{0};
    opengame::LogLevel last_level{opengame::LogLevel::debug};
    std::string_view last_message{};
};

void capture_log(opengame::LogLevel level,
                 std::string_view message,
                 void* user_data) noexcept {
    auto* capture = static_cast<LogCapture*>(user_data);
    ++capture->entries;
    capture->last_level = level;
    capture->last_message = message;
}

} // namespace

int main() {
    using namespace opengame;

    DeviceProfile device{};
    device.cpu_architecture = CpuArchitecture::arm64;
    device.cpu_cores = 8;
    device.total_memory_bytes = 8ULL * 1024ULL * 1024ULL * 1024ULL;
    device.android_api_level = 35;
    device.vulkan_supported = true;
    device.opengl_es_major = 3;
    device.opengl_es_minor = 2;

    SessionConfig config{};
    config.engine = EngineKind::pc;
    config.content_directory = "test-content";
    config.render_width = 1920;
    config.render_height = 1080;

    LogCapture capture{};
    Logger logger{capture_log, &capture};

    Core core;
    assert(core.state() == SessionState::stopped);
    assert(core.version() == "0.1.1");
    assert(!core.start());

    DeviceProfile invalid_device{};
    assert(!core.initialize(invalid_device, config, logger));
    assert(core.state() == SessionState::stopped);
    assert(capture.entries == 1);
    assert(capture.last_level == LogLevel::error);

    assert(core.initialize(device, config, logger));
    assert(core.state() == SessionState::initialized);
    assert(core.device_profile().cpu_cores == 8);
    assert(core.session_config().render_width == 1920);
    assert(capture.last_level == LogLevel::info);

    // No real PC/PS3 backend exists yet: start must not report a running session.
    assert(!core.start());
    assert(core.state() == SessionState::initialized);
    assert(capture.last_level == LogLevel::warning);

    core.stop();
    assert(core.state() == SessionState::stopped);
    assert(capture.last_level == LogLevel::info);
    return 0;
}
