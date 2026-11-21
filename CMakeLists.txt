cmake_minimum_required(VERSION 3.22.1)
project(Opengame LANGUAGES CXX)

option(OPENGAME_BUILD_TESTS "Build Opengame unit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(core/opengame-core)
add_subdirectory(core/pc-engine)

if(OPENGAME_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
