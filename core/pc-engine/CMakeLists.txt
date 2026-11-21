add_library(opengame_pc_engine STATIC
    src/pe_inspector.cpp
)

target_include_directories(opengame_pc_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(opengame_pc_engine PUBLIC cxx_std_20)
