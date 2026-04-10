cmake_minimum_required(VERSION 3.25)
project(HoroEngineStarter VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(MSVC)
    add_compile_options(/W4 /WX /Zc:preprocessor)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(HORO_ENGINE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine")
if(NOT EXISTS "${HORO_ENGINE_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "Could not find horo-engine source. Run 'git submodule update --init --recursive'.")
endif()

add_subdirectory(${HORO_ENGINE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/horo-engine-build)
add_subdirectory(app)
