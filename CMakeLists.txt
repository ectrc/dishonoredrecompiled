cmake_minimum_required(VERSION 3.25)
project(Dishonored LANGUAGES CXX)

if(NOT MSVC)
  message(FATAL_ERROR "Dishonored builds with MSVC only")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "Configure inside a VS x86 environment (vcvars32); pointer size must be 4")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_compile_options(/W4 /permissive- /Zc:__cplusplus /fp:precise /EHsc /utf-8 /MP)
add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)

set(DISHONORED_BINARIES_DIR "${CMAKE_BINARY_DIR}/Binaries/Win32")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DISHONORED_BINARIES_DIR}")

add_subdirectory(Development/Src/Launch)
