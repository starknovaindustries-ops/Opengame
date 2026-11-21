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
void put(std::vector<std::byte>& b, std::size_t o, const char* s) {
    for(std::size_t i=0;;++i){ b[o+i]=static_cast<std::byte>(s[i]); if(s[i]=='\\0') break; }
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
    // Import directory fixture.
    w32(b,0x200,0x1060); w32(b,0x20c,0x1040); w32(b,0x210,0x1070);
    put(b,0x240,"KERNEL32.dll");
    w64(b,0x260,0x1080); w64(b,0x268,0);
    w16(b,0x280,0); put(b,0x282,"CreateFileW");
    return b;
}
void add_exports(std::vector<std::byte>& b) {
    // Export directory at RVA 0x1100 -> file offset 0x300.
    w32(b,0x98+112,0x1100); w32(b,0x98+112+4,0x80);
    w32(b,0x30c,0x1140); // DLL name
    w32(b,0x310,1);      // ordinal base
    w32(b,0x314,2);      // number of functions
    w32(b,0x318,1);      // number of names
    w32(b,0x31c,0x1150); // function RVA array
    w32(b,0x320,0x1158); // name RVA array
    w32(b,0x324,0x115c); // ordinal array
    put(b,0x340,"sample.dll");
    w32(b,0x350,0x1180); // named function
    w32(b,0x354,0x1170); // forwarder
    w32(b,0x358,0x1160);
    w16(b,0x35c,0);
    put(b,0x360,"OpenGame");
    put(b,0x370,"OTHER.Func");
    put(b,0x380,"NamedExport");
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

    auto with_exports=make_pe();
    add_exports(with_exports);
    auto exports=inspect_pe_exports(with_exports);
    assert(exports && exports.symbol_count==2);
    assert(exports.module_name[0]=='s');
    assert(exports.symbols[0].has_name);
    assert(exports.symbols[0].name[0]=='N');
    assert(exports.symbols[0].ordinal==1);
    assert(exports.symbols[1].is_forwarder);
    assert(exports.symbols[1].forwarder[0]=='O');
    auto bad_exports=with_exports;
    w32(bad_exports,0x98+112+4,0x7fffffff);
    assert(inspect_pe_exports(bad_exports).error==PeExportError::invalid_rva);
    auto no_exports=make_pe();
    assert(inspect_pe_exports(no_exports).error==PeExportError::no_export_directory);

    // Dependency analysis: app.exe imports NamedExport from the supplied sample.dll.
    auto app_image = make_pe();
    put(app_image, 0x240, "sample.dll");
    put(app_image, 0x282, "NamedExport");
    auto provider_image = make_pe();
    add_exports(provider_image);
    PeDependencyInput dependency_inputs[2]{};
    const char app_name[] = "app.exe";
    const char dll_name[] = "sample.dll";
    for (std::size_t i = 0; i < sizeof(app_name); ++i)
        dependency_inputs[0].file_name[i] = app_name[i];
    for (std::size_t i = 0; i < sizeof(dll_name); ++i)
        dependency_inputs[1].file_name[i] = dll_name[i];
    dependency_inputs[0].bytes = app_image;
    dependency_inputs[1].bytes = provider_image;
    const auto dependency_report = analyze_pe_dependencies(dependency_inputs);
    assert(dependency_report && dependency_report.image_count == 2);
    assert(dependency_report.images[0].module_count == 1);
    assert(dependency_report.images[0].modules[0].status == PeDependencyStatus::symbols_resolved);
    assert(dependency_report.images[0].modules[0].resolved_symbol_count == 1);
    assert(dependency_report.images[0].modules[0].missing_symbol_count == 0);

    put(app_image, 0x240, "missing.dll");
    dependency_inputs[0].bytes = app_image;
    const auto missing_module_report = analyze_pe_dependencies(dependency_inputs);
    assert(missing_module_report);
    assert(missing_module_report.images[0].modules[0].status == PeDependencyStatus::module_not_found);

    put(app_image, 0x240, "sample.dll");
    put(app_image, 0x282, "NoSuchFunc");
    dependency_inputs[0].bytes = app_image;
    const auto missing_symbol_report = analyze_pe_dependencies(dependency_inputs);
    assert(missing_symbol_report);
    assert(missing_symbol_report.images[0].modules[0].status == PeDependencyStatus::symbols_missing);
    assert(missing_symbol_report.images[0].modules[0].missing_symbol_count == 1);

    PeDependencyInput too_many[pe_max_dependency_images + 1]{};
    const auto limit_report = analyze_pe_dependencies(too_many);
    assert(limit_report.error == PeDependencyError::too_many_images);

    return 0;
}
