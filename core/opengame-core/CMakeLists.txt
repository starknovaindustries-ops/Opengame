add_library(opengame_core STATIC
    src/opengame_core.cpp
)

target_include_directories(opengame_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(opengame_core PUBLIC cxx_std_20)
