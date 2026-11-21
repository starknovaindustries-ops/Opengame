#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace opengame::pc {

enum class PeMachine : std::uint16_t {
    unknown = 0, x86 = 0x014c, arm = 0x01c0, x64 = 0x8664, arm64 = 0xaa64
};
enum class PeFormat : std::uint16_t { unknown = 0, pe32 = 0x010b, pe32_plus = 0x020b };
enum class PeError {
    none, too_small, invalid_dos_signature, invalid_pe_offset,
    invalid_pe_signature, invalid_optional_header, invalid_section_table
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
    [[nodiscard]] explicit operator bool() const noexcept { return error == PeError::none; }
};

enum class PeImportError {
    none, invalid_image, no_import_directory, invalid_directory,
    invalid_rva, invalid_string, limit_exceeded
};

inline constexpr std::size_t pe_name_capacity = 128;
inline constexpr std::size_t pe_max_import_modules = 32;
inline constexpr std::size_t pe_max_symbols_per_module = 64;

struct PeImportedSymbol {
    std::array<char, pe_name_capacity> name{};
    std::uint16_t ordinal{0};
    bool by_ordinal{false};
};
struct PeImportModule {
    std::array<char, pe_name_capacity> name{};
    std::array<PeImportedSymbol, pe_max_symbols_per_module> symbols{};
    std::size_t symbol_count{0};
};
struct PeImportInspection {
    PeImportError error{PeImportError::none};
    std::array<PeImportModule, pe_max_import_modules> modules{};
    std::size_t module_count{0};
    [[nodiscard]] explicit operator bool() const noexcept { return error == PeImportError::none; }
};

// Static metadata inspection only. Neither API loads or executes the image.
[[nodiscard]] PeInspection inspect_pe(std::span<const std::byte> bytes) noexcept;
[[nodiscard]] PeImportInspection inspect_pe_imports(std::span<const std::byte> bytes) noexcept;

} // namespace opengame::pc
