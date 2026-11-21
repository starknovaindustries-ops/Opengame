add_executable(opengame_core_tests
    core_tests.cpp
)

target_link_libraries(opengame_core_tests PRIVATE opengame_core)
add_test(NAME opengame_core_tests COMMAND opengame_core_tests)

add_executable(opengame_pe_inspector_tests
    pe_inspector_tests.cpp
)

target_link_libraries(opengame_pe_inspector_tests PRIVATE opengame_pc_engine)
add_test(NAME opengame_pe_inspector_tests COMMAND opengame_pe_inspector_tests)
