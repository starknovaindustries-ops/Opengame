#include "pe_inspector.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

void write_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xffU);
    bytes[offset + 1] = static_cast<std::byte>((value >> 8) & 0xffU);
}

void write_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (unsigned int i = 0; i < 4; ++i) {
        bytes[offset + i] = static_cast<std::byte>((value >> (i * 8)) & 0xffU);
    }
}

std::vector<std::byte> make_pe(std::uint16_t machine,
                               std::uint16_t optional_magic,
                               std::uint16_t characteristics = 0x0002U) {
    std::vector<std::byte> bytes(512);
    write_u16(bytes, 0, 0x5a4d);
    write_u32(bytes, 0x3c, 0x80);
    write_u32(bytes, 0x80, 0x00004550);
    write_u16(bytes, 0x84, machine);
    write_u16(bytes, 0x86, 1);
    write_u16(bytes, 0x94, 0x00f0);
    write_u16(bytes, 0x96, characteristics);
    write_u16(bytes, 0x98, optional_magic);
    return bytes;
}

} // namespace

int main() {
    using namespace opengame::pc;

    const auto x64 = make_pe(0x8664, 0x020b);
    const auto x64_result = inspect_pe(x64);
    assert(x64_result);
    assert(x64_result.info.machine == PeMachine::x64);
    assert(x64_result.info.format == PeFormat::pe32_plus);
    assert(x64_result.info.section_count == 1);
    assert(x64_result.info.is_executable);
    assert(!x64_result.info.is_dll);

    const auto x86 = make_pe(0x014c, 0x010b, 0x2000U);
    const auto x86_result = inspect_pe(x86);
    assert(x86_result);
    assert(x86_result.info.machine == PeMachine::x86);
    assert(x86_result.info.format == PeFormat::pe32);
    assert(x86_result.info.is_dll);

    const std::vector<std::byte> empty;
    assert(inspect_pe(empty).error == PeError::too_small);

    auto bad_dos = x64;
    write_u16(bad_dos, 0, 0);
    assert(inspect_pe(bad_dos).error == PeError::invalid_dos_signature);

    auto bad_pe = x64;
    write_u32(bad_pe, 0x80, 0);
    assert(inspect_pe(bad_pe).error == PeError::invalid_pe_signature);

    auto bad_offset = x64;
    write_u32(bad_offset, 0x3c, 0xfffffff0U);
    assert(inspect_pe(bad_offset).error == PeError::invalid_pe_offset);

    auto bad_optional = x64;
    write_u16(bad_optional, 0x98, 0x9999);
    assert(inspect_pe(bad_optional).error == PeError::invalid_optional_header);

    auto bad_sections = x64;
    write_u16(bad_sections, 0x86, 97);
    assert(inspect_pe(bad_sections).error == PeError::invalid_section_table);

    return 0;
}
