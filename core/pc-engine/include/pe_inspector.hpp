#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace opengame::pc {

enum class PeMachine : std::uint16_t {
    unknown = 0,
    x86 = 0x014c,
    arm = 0x01c0,
    x64 = 0x8664,
    arm64 = 0xaa64
};

enum class PeFormat : std::uint16_t {
    unknown = 0,
    pe32 = 0x010b,
    pe32_plus = 0x020b
};

enum class PeError {
    none,
    too_small,
    invalid_dos_signature,
    invalid_pe_offset,
    invalid_pe_signature,
    invalid_optional_header,
    invalid_section_table
};

struct PeImageInfo {
    PeMachine machine{PeMachine::unknown};
    PeFormat format{PeFormat::unknown};
    std::uint16_t section_count{0};
    bool is_executable{false};
    bool is_dll{false};
};

struct PeInspection {
    PeError error{PeError::none};
    PeImageInfo info{};
    [[nodiscard]] explicit operator bool() const noexcept {
        return error == PeError::none;
    }
};

// Inspects metadata only. It never loads or executes the image.
[[nodiscard]] PeInspection inspect_pe(std::span<const std::byte> bytes) noexcept;

} // namespace opengame::pc
