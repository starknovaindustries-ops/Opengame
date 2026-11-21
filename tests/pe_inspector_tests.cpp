#include "pe_inspector.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {
void w16(std::vector<std::byte>& b, std::size_t o, std::uint16_t v) {
    b[o]=static_cast<std::byte>(v&255U); b[o+1]=static_cast<std::byte>((v>>8)&255U);
}
void w32(std::vector<std::byte>& b, std::size_t o, std::uint32_t v) {
    for(unsigned i=0;i<4;++i) b[o+i]=static_cast<std::byte>((v>>(i*8))&255U);
}
void w64(std::vector<std::byte>& b, std::size_t o, std::uint64_t v) {
    for(unsigned i=0;i<8;++i) b[o+i]=static_cast<std::byte>((v>>(i*8))&255U);
}
std::vector<std::byte> make_pe() {
    std::vector<std::byte> b(1024);
    w16(b,0,0x5a4d); w32(b,0x3c,0x80); w32(b,0x80,0x4550);
    w16(b,0x84,0x8664); w16(b,0x86,1); w16(b,0x94,0x00f0);
    w16(b,0x98,0x020b); w32(b,0x98+108,16);
    w32(b,0x98+112+8,0x1000); w32(b,0x98+112+12,40);
    const std::size_t sh=0x188;
    w32(b,sh+8,0x200); w32(b,sh+12,0x1000);
    w32(b,sh+16,0x200); w32(b,sh+20,0x200);
    // Import descriptor, followed by its null terminator.
    w32(b,0x200,0x1060); w32(b,0x20c,0x1040); w32(b,0x210,0x1070);
    const char dll[]="KERNEL32.dll";
    for(std::size_t i=0;i<sizeof(dll);++i)b[0x240+i]=static_cast<std::byte>(dll[i]);
    w64(b,0x260,0x1080); w64(b,0x268,0);
    w16(b,0x280,0);
    const char fn[]="CreateFileW";
    for(std::size_t i=0;i<sizeof(fn);++i)b[0x282+i]=static_cast<std::byte>(fn[i]);
    return b;
}
}
int main() {
    using namespace opengame::pc;
    auto b=make_pe();
    auto meta=inspect_pe(b);
    assert(meta && meta.info.machine==PeMachine::x64);
    auto imports=inspect_pe_imports(b);
    assert(imports && imports.module_count==1);
    assert(imports.modules[0].name[0]=='K');
    assert(imports.modules[0].symbol_count==1);
    assert(imports.modules[0].symbols[0].name[0]=='C');
    auto invalid=b;
    w32(invalid,0x98+112+8,0x7fffffff);
    assert(inspect_pe_imports(invalid).error==PeImportError::invalid_rva);
    auto no_directory=b;
    w32(no_directory,0x98+112+8,0);
    w32(no_directory,0x98+112+12,0);
    assert(inspect_pe_imports(no_directory).error==PeImportError::no_import_directory);
    auto truncated=b;
    truncated.resize(0x250);
    assert(inspect_pe_imports(truncated).error!=PeImportError::none);
    return 0;
}
