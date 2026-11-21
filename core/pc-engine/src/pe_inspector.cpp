#include "pe_inspector.hpp"

#include <limits>

namespace opengame::pc {
namespace {
constexpr std::size_t kDosMinimum = 0x40;
constexpr std::size_t kPeSignatureSize = 4;
constexpr std::size_t kCoffSize = 20;
constexpr std::size_t kSectionHeaderSize = 40;
constexpr std::uint16_t kMaxSections = 96;
constexpr std::size_t kImportDescriptorSize = 20;
constexpr std::size_t kExportDirectorySize = 40;

struct Layout {
    std::size_t optional_offset{};
    std::size_t optional_size{};
    std::size_t section_table_offset{};
    std::uint16_t section_count{};
    PeFormat format{PeFormat::unknown};
    std::size_t directory_offset{};
    std::uint32_t directory_count{};
};
struct Section {
    std::uint32_t virtual_size{};
    std::uint32_t virtual_address{};
    std::uint32_t raw_size{};
    std::uint32_t raw_offset{};
};
[[nodiscard]] bool has_range(std::size_t size, std::size_t offset, std::size_t length) noexcept {
    return offset <= size && length <= size - offset;
}
[[nodiscard]] std::uint16_t read_u16(std::span<const std::byte> b, std::size_t o) noexcept {
    return static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(b[o]) |
        (static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(b[o + 1])) << 8));
}
[[nodiscard]] std::uint32_t read_u32(std::span<const std::byte> b, std::size_t o) noexcept {
    std::uint32_t v = 0;
    for (unsigned i = 0; i < 4; ++i)
        v |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(b[o + i])) << (i * 8);
    return v;
}
[[nodiscard]] std::uint64_t read_u64(std::span<const std::byte> b, std::size_t o) noexcept {
    std::uint64_t v = 0;
    for (unsigned i = 0; i < 8; ++i)
        v |= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(b[o + i])) << (i * 8);
    return v;
}
[[nodiscard]] PeMachine decode_machine(std::uint16_t v) noexcept {
    switch (v) {
        case 0x014c: return PeMachine::x86;
        case 0x01c0: return PeMachine::arm;
        case 0x8664: return PeMachine::x64;
        case 0xaa64: return PeMachine::arm64;
        default: return PeMachine::unknown;
    }
}
[[nodiscard]] bool parse_layout(std::span<const std::byte> b, Layout& out) noexcept {
    if (b.size() < kDosMinimum || read_u16(b, 0) != 0x5a4d) return false;
    const auto pe = static_cast<std::size_t>(read_u32(b, 0x3c));
    if (!has_range(b.size(), pe, kPeSignatureSize + kCoffSize) ||
        read_u32(b, pe) != 0x00004550) return false;
    const auto coff = pe + kPeSignatureSize;
    const auto optional_size = read_u16(b, coff + 16);
    const auto optional = coff + kCoffSize;
    if (optional_size < 2 || !has_range(b.size(), optional, optional_size)) return false;
    const auto magic = read_u16(b, optional);
    if (magic == 0x010b) {
        out.format = PeFormat::pe32;
        out.directory_offset = optional + 96;
        if (optional_size < 96) return false;
        out.directory_count = read_u32(b, optional + 92);
    } else if (magic == 0x020b) {
        out.format = PeFormat::pe32_plus;
        out.directory_offset = optional + 112;
        if (optional_size < 112) return false;
        out.directory_count = read_u32(b, optional + 108);
    } else return false;
    out.optional_offset = optional;
    out.optional_size = optional_size;
    out.section_count = read_u16(b, coff + 2);
    if (out.section_count > kMaxSections) return false;
    out.section_table_offset = optional + optional_size;
    return has_range(b.size(), out.section_table_offset,
                     static_cast<std::size_t>(out.section_count) * kSectionHeaderSize);
}
[[nodiscard]] bool read_section(std::span<const std::byte> b, const Layout& l,
                                std::size_t index, Section& s) noexcept {
    if (index >= l.section_count) return false;
    const auto o = l.section_table_offset + index * kSectionHeaderSize;
    s.virtual_size = read_u32(b, o + 8);
    s.virtual_address = read_u32(b, o + 12);
    s.raw_size = read_u32(b, o + 16);
    s.raw_offset = read_u32(b, o + 20);
    return true;
}
[[nodiscard]] bool map_rva(std::span<const std::byte> b, const Layout& l,
                           std::uint32_t rva, std::size_t length,
                           std::size_t& offset) noexcept {
    for (std::size_t i = 0; i < l.section_count; ++i) {
        Section s{};
        if (!read_section(b, l, i, s)) return false;
        const auto extent = s.virtual_size > s.raw_size ? s.virtual_size : s.raw_size;
        if (rva < s.virtual_address || static_cast<std::uint64_t>(rva) >=
            static_cast<std::uint64_t>(s.virtual_address) + extent) continue;
        const auto delta = static_cast<std::uint64_t>(rva) - s.virtual_address;
        if (delta > s.raw_size || length > s.raw_size - static_cast<std::size_t>(delta))
            return false;
        const auto raw = static_cast<std::uint64_t>(s.raw_offset) + delta;
        if (raw > std::numeric_limits<std::size_t>::max()) return false;
        offset = static_cast<std::size_t>(raw);
        return has_range(b.size(), offset, length);
    }
    return false;
}
[[nodiscard]] bool copy_c_string(std::span<const std::byte> b, std::size_t offset,
                                 std::array<char, pe_name_capacity>& out) noexcept {
    if (offset >= b.size()) return false;
    for (std::size_t i = 0; i < out.size(); ++i) {
        if (offset + i >= b.size()) return false;
        const auto ch = std::to_integer<unsigned char>(b[offset + i]);
        if (ch == 0) return i != 0;
        out[i] = static_cast<char>(ch);
    }
    return false;
}
[[nodiscard]] bool copy_rva_string(std::span<const std::byte> b, const Layout& l,
                                   std::uint32_t rva,
                                   std::array<char, pe_name_capacity>& out) noexcept {
    for (std::size_t i = 0; i < out.size(); ++i) {
        const auto current = static_cast<std::uint64_t>(rva) + i;
        std::size_t offset{};
        if (current > UINT32_MAX ||
            !map_rva(b, l, static_cast<std::uint32_t>(current), 1, offset)) return false;
        const auto ch = std::to_integer<unsigned char>(b[offset]);
        if (ch == 0) return i != 0;
        out[i] = static_cast<char>(ch);
    }
    return false;
}
} // namespace

PeInspection inspect_pe(std::span<const std::byte> bytes) noexcept {
    Layout l{};
    if (bytes.size() < kDosMinimum) return {PeError::too_small, {}};
    if (read_u16(bytes, 0) != 0x5a4d) return {PeError::invalid_dos_signature, {}};
    const auto pe = static_cast<std::size_t>(read_u32(bytes, 0x3c));
    if (!has_range(bytes.size(), pe, kPeSignatureSize + kCoffSize))
        return {PeError::invalid_pe_offset, {}};
    if (read_u32(bytes, pe) != 0x00004550) return {PeError::invalid_pe_signature, {}};
    const auto coff = pe + kPeSignatureSize;
    const auto optional_size = read_u16(bytes, coff + 16);
    const auto optional = coff + kCoffSize;
    if (optional_size < 2 || !has_range(bytes.size(), optional, optional_size))
        return {PeError::invalid_optional_header, {}};
    const auto magic = read_u16(bytes, optional);
    PeFormat format = magic == 0x010b ? PeFormat::pe32 :
                      magic == 0x020b ? PeFormat::pe32_plus : PeFormat::unknown;
    if (format == PeFormat::unknown) return {PeError::invalid_optional_header, {}};
    const auto sections = read_u16(bytes, coff + 2);
    const auto table = optional + optional_size;
    if (sections > kMaxSections ||
        !has_range(bytes.size(), table, static_cast<std::size_t>(sections) * kSectionHeaderSize))
        return {PeError::invalid_section_table, {}};
    PeImageInfo info{};
    info.machine = decode_machine(read_u16(bytes, coff));
    info.format = format;
    info.section_count = sections;
    const auto flags = read_u16(bytes, coff + 18);
    info.is_executable = (flags & 0x0002U) != 0;
    info.is_dll = (flags & 0x2000U) != 0;
    return {PeError::none, info};
}

PeImportInspection inspect_pe_imports(std::span<const std::byte> bytes) noexcept {
    Layout l{};
    if (!parse_layout(bytes, l)) return {PeImportError::invalid_image, {}, 0};
    if (l.directory_count <= 1 || l.optional_size < (l.directory_offset - l.optional_offset) + 16)
        return {PeImportError::no_import_directory, {}, 0};
    const auto import_rva = read_u32(bytes, l.directory_offset + 8);
    const auto import_size = read_u32(bytes, l.directory_offset + 12);
    if (import_rva == 0 && import_size == 0) return {PeImportError::no_import_directory, {}, 0};
    if (import_rva == 0 || import_size < kImportDescriptorSize)
        return {PeImportError::invalid_directory, {}, 0};
    PeImportInspection result{};
    const auto descriptor_count = import_size / kImportDescriptorSize;
    const auto bounded_descriptors = descriptor_count < pe_max_import_modules + 1
        ? descriptor_count : pe_max_import_modules + 1;
    bool terminated = false;
    for (std::size_t d = 0; d < bounded_descriptors; ++d) {
        std::size_t descriptor_offset{};
        const auto descriptor_rva64 = static_cast<std::uint64_t>(import_rva) + d * kImportDescriptorSize;
        if (descriptor_rva64 > UINT32_MAX ||
            !map_rva(bytes, l, static_cast<std::uint32_t>(descriptor_rva64),
                     kImportDescriptorSize, descriptor_offset))
            return {PeImportError::invalid_rva, {}, 0};
        const auto original_thunk = read_u32(bytes, descriptor_offset);
        const auto name_rva = read_u32(bytes, descriptor_offset + 12);
        const auto first_thunk = read_u32(bytes, descriptor_offset + 16);
        if (original_thunk == 0 && name_rva == 0 && first_thunk == 0) {
            terminated = true;
            break;
        }
        if (result.module_count >= pe_max_import_modules)
            return {PeImportError::limit_exceeded, {}, 0};
        auto& module = result.modules[result.module_count];
        std::size_t name_offset{};
        if (name_rva == 0 || !map_rva(bytes, l, name_rva, 1, name_offset) ||
            !copy_c_string(bytes, name_offset, module.name))
            return {PeImportError::invalid_string, {}, 0};
        const auto thunk_rva = original_thunk != 0 ? original_thunk : first_thunk;
        if (thunk_rva == 0) return {PeImportError::invalid_directory, {}, 0};
        const std::size_t width = l.format == PeFormat::pe32_plus ? 8 : 4;
        bool thunk_terminated = false;
        for (std::size_t t = 0; t <= pe_max_symbols_per_module; ++t) {
            const auto entry_rva64 = static_cast<std::uint64_t>(thunk_rva) + t * width;
            std::size_t entry_offset{};
            if (entry_rva64 > UINT32_MAX ||
                !map_rva(bytes, l, static_cast<std::uint32_t>(entry_rva64), width, entry_offset))
                return {PeImportError::invalid_rva, {}, 0};
            const std::uint64_t value = width == 8 ? read_u64(bytes, entry_offset) :
                                                      read_u32(bytes, entry_offset);
            if (value == 0) { thunk_terminated = true; break; }
            if (t >= pe_max_symbols_per_module) return {PeImportError::limit_exceeded, {}, 0};
            auto& symbol = module.symbols[module.symbol_count];
            const std::uint64_t ordinal_mask = width == 8 ? 0x8000000000000000ULL : 0x80000000ULL;
            if ((value & ordinal_mask) != 0) {
                symbol.by_ordinal = true;
                symbol.ordinal = static_cast<std::uint16_t>(value & 0xffffU);
            } else {
                if (value > UINT32_MAX) return {PeImportError::invalid_rva, {}, 0};
                std::size_t hint_offset{};
                if (!map_rva(bytes, l, static_cast<std::uint32_t>(value), 2, hint_offset) ||
                    !copy_c_string(bytes, hint_offset + 2, symbol.name))
                    return {PeImportError::invalid_string, {}, 0};
            }
            ++module.symbol_count;
        }
        if (!thunk_terminated) return {PeImportError::limit_exceeded, {}, 0};
        ++result.module_count;
    }
    if (!terminated) return {PeImportError::limit_exceeded, {}, 0};
    return result;
}

PeExportInspection inspect_pe_exports(std::span<const std::byte> bytes) noexcept {
    Layout l{};
    if (!parse_layout(bytes, l)) return {PeExportError::invalid_image, {}, {}, 0};
    if (l.directory_count == 0 || l.optional_size < (l.directory_offset - l.optional_offset) + 8)
        return {PeExportError::no_export_directory, {}, {}, 0};
    const auto export_rva = read_u32(bytes, l.directory_offset);
    const auto export_size = read_u32(bytes, l.directory_offset + 4);
    if (export_rva == 0 && export_size == 0)
        return {PeExportError::no_export_directory, {}, {}, 0};
    if (export_rva == 0 || export_size < kExportDirectorySize)
        return {PeExportError::invalid_directory, {}, {}, 0};
    std::size_t directory_offset{};
    if (!map_rva(bytes, l, export_rva, kExportDirectorySize, directory_offset))
        return {PeExportError::invalid_rva, {}, {}, 0};

    const auto name_rva = read_u32(bytes, directory_offset + 12);
    const auto ordinal_base = read_u32(bytes, directory_offset + 16);
    const auto function_count = read_u32(bytes, directory_offset + 20);
    const auto name_count = read_u32(bytes, directory_offset + 24);
    const auto functions_rva = read_u32(bytes, directory_offset + 28);
    const auto names_rva = read_u32(bytes, directory_offset + 32);
    const auto ordinals_rva = read_u32(bytes, directory_offset + 36);
    if (function_count > pe_max_exports || name_count > function_count)
        return {PeExportError::limit_exceeded, {}, {}, 0};
    if (name_rva != 0) {
        std::size_t ignored{};
        if (!map_rva(bytes, l, name_rva, 1, ignored))
            return {PeExportError::invalid_rva, {}, {}, 0};
    }
    if (function_count != 0 && functions_rva == 0)
        return {PeExportError::invalid_directory, {}, {}, 0};
    if (name_count != 0 && (names_rva == 0 || ordinals_rva == 0))
        return {PeExportError::invalid_directory, {}, {}, 0};

    PeExportInspection result{};
    if (name_rva != 0 && !copy_rva_string(bytes, l, name_rva, result.module_name))
        return {PeExportError::invalid_string, {}, {}, 0};

    std::array<std::uint32_t, pe_max_exports> name_rvas{};
    std::array<std::uint16_t, pe_max_exports> name_ordinals{};
    std::array<bool, pe_max_exports> has_name{};
    if (name_count != 0) {
        std::size_t names_offset{}, ordinals_offset{};
        const auto names_bytes = static_cast<std::size_t>(name_count) * 4;
        const auto ordinals_bytes = static_cast<std::size_t>(name_count) * 2;
        if (!map_rva(bytes, l, names_rva, names_bytes, names_offset) ||
            !map_rva(bytes, l, ordinals_rva, ordinals_bytes, ordinals_offset))
            return {PeExportError::invalid_rva, {}, {}, 0};
        for (std::size_t i = 0; i < name_count; ++i) {
            const auto index = read_u16(bytes, ordinals_offset + i * 2);
            if (index >= function_count || has_name[index])
                return {PeExportError::invalid_directory, {}, {}, 0};
            name_rvas[i] = read_u32(bytes, names_offset + i * 4);
            name_ordinals[i] = index;
            has_name[index] = true;
        }
    }
    std::array<std::array<char, pe_name_capacity>, pe_max_exports> names{};
    for (std::size_t i = 0; i < name_count; ++i) {
        if (name_rvas[i] == 0 || !copy_rva_string(bytes, l, name_rvas[i], names[name_ordinals[i]]))
            return {PeExportError::invalid_string, {}, {}, 0};
    }

    std::size_t functions_offset{};
    if (function_count != 0 &&
        !map_rva(bytes, l, functions_rva, static_cast<std::size_t>(function_count) * 4,
                 functions_offset))
        return {PeExportError::invalid_rva, {}, {}, 0};
    for (std::size_t i = 0; i < function_count; ++i) {
        const auto function_rva = read_u32(bytes, functions_offset + i * 4);
        if (function_rva == 0) continue;
        auto& symbol = result.symbols[result.symbol_count++];
        const auto ordinal64 = static_cast<std::uint64_t>(ordinal_base) + i;
        if (ordinal64 > UINT32_MAX) return {PeExportError::invalid_directory, {}, {}, 0};
        symbol.ordinal = static_cast<std::uint32_t>(ordinal64);
        symbol.function_rva = function_rva;
        symbol.has_name = has_name[i];
        if (symbol.has_name) symbol.name = names[i];
        const auto export_end = static_cast<std::uint64_t>(export_rva) + export_size;
        if (function_rva >= export_rva && static_cast<std::uint64_t>(function_rva) < export_end) {
            symbol.is_forwarder = true;
            if (!copy_rva_string(bytes, l, function_rva, symbol.forwarder))
                return {PeExportError::invalid_string, {}, {}, 0};
        }
    }
    return result;
}
} // namespace opengame::pc
