#include "pe_inspector.hpp"

#include <limits>

namespace opengame::pc {
namespace {

[[nodiscard]] bool has_range(std::size_t size,
                             std::size_t offset,
                             std::size_t length) noexcept {
    return offset <= size && length <= size - offset;
}

[[nodiscard]] std::uint16_t read_u16(std::span<const std::byte> bytes,
                                     std::size_t offset) noexcept {
    const auto low = std::to_integer<std::uint8_t>(bytes[offset]);
    const auto high = std::to_integer<std::uint8_t>(bytes[offset + 1]);
    return static_cast<std::uint16_t>(low | (static_cast<std::uint16_t>(high) << 8));
}

[[nodiscard]] std::uint32_t read_u32(std::span<const std::byte> bytes,
                                     std::size_t offset) noexcept {
    std::uint32_t value = 0;
    for (unsigned int i = 0; i < 4; ++i) {
        value |= static_cast<std::uint32_t>(
                     std::to_integer<std::uint8_t>(bytes[offset + i]))
                 << (i * 8);
    }
    return value;
}

[[nodiscard]] PeMachine decode_machine(std::uint16_t value) noexcept {
    switch (value) {
        case 0x014c: return PeMachine::x86;
        case 0x01c0: return PeMachine::arm;
        case 0x8664: return PeMachine::x64;
        case 0xaa64: return PeMachine::arm64;
        default: return PeMachine::unknown;
    }
}

} // namespace

PeInspection inspect_pe(std::span<const std::byte> bytes) noexcept {
    constexpr std::size_t dos_header_minimum = 0x40;
    constexpr std::size_t pe_signature_size = 4;
    constexpr std::size_t coff_header_size = 20;
    constexpr std::size_t optional_magic_size = 2;
    constexpr std::size_t section_header_size = 40;
    constexpr std::uint16_t max_sections = 96;

    if (bytes.size() < dos_header_minimum) {
        return {PeError::too_small, {}};
    }

    if (read_u16(bytes, 0) != 0x5a4d) {
        return {PeError::invalid_dos_signature, {}};
    }

    const std::uint32_t pe_offset_raw = read_u32(bytes, 0x3c);
    if (pe_offset_raw > std::numeric_limits<std::size_t>::max()) {
        return {PeError::invalid_pe_offset, {}};
    }
    const auto pe_offset = static_cast<std::size_t>(pe_offset_raw);

    if (!has_range(bytes.size(), pe_offset, pe_signature_size + coff_header_size)) {
        return {PeError::invalid_pe_offset, {}};
    }

    if (read_u32(bytes, pe_offset) != 0x00004550) {
        return {PeError::invalid_pe_signature, {}};
    }

    const auto coff = pe_offset + pe_signature_size;
    const auto machine = read_u16(bytes, coff);
    const auto section_count = read_u16(bytes, coff + 2);
    const auto optional_size = read_u16(bytes, coff + 16);
    const auto characteristics = read_u16(bytes, coff + 18);
    const auto optional_offset = coff + coff_header_size;

    if (optional_size < optional_magic_size ||
        !has_range(bytes.size(), optional_offset, optional_size)) {
        return {PeError::invalid_optional_header, {}};
    }

    const auto magic = read_u16(bytes, optional_offset);
    PeFormat format = PeFormat::unknown;
    if (magic == static_cast<std::uint16_t>(PeFormat::pe32)) {
        format = PeFormat::pe32;
    } else if (magic == static_cast<std::uint16_t>(PeFormat::pe32_plus)) {
        format = PeFormat::pe32_plus;
    } else {
        return {PeError::invalid_optional_header, {}};
    }

    if (section_count > max_sections) {
        return {PeError::invalid_section_table, {}};
    }

    const auto section_table_offset = optional_offset + optional_size;
    const auto section_bytes = static_cast<std::size_t>(section_count) * section_header_size;
    if (!has_range(bytes.size(), section_table_offset, section_bytes)) {
        return {PeError::invalid_section_table, {}};
    }

    PeImageInfo info{};
    info.machine = decode_machine(machine);
    info.format = format;
    info.section_count = section_count;
    info.is_executable = (characteristics & 0x0002U) != 0;
    info.is_dll = (characteristics & 0x2000U) != 0;
    return {PeError::none, info};
}

} // namespace opengame::pc
