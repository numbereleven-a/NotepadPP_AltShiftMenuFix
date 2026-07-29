cmake_minimum_required(VERSION 3.21)

project(
    NppAltShiftMenuFix
    VERSION 1.0
    DESCRIPTION "Prevent accidental Notepad++ menu activation after Alt+Shift"
    LANGUAGES CXX RC)

include(CTest)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "NppAltShiftMenuFix currently supports Windows x64 only")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(ASMF_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
if(ASMF_VERSION_PATCH STREQUAL "")
    set(ASMF_VERSION_PATCH 0)
endif()

configure_file(cmake/Version.h.in generated/Version.h @ONLY)
configure_file(resources/AltShiftMenuFix.rc.in generated/AltShiftMenuFix.rc @ONLY)
configure_file(resources/TestHost.rc.in generated/TestHost.rc @ONLY)

add_library(AltShiftMenuFix SHARED
    src/AltShiftMenuFix.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/generated/AltShiftMenuFix.rc)

target_include_directories(AltShiftMenuFix PRIVATE
    include
    ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_compile_features(AltShiftMenuFix PRIVATE cxx_std_17)
target_compile_definitions(AltShiftMenuFix PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WIN32_WINNT=0x0601
    WINVER=0x0601)
target_link_libraries(AltShiftMenuFix PRIVATE comctl32 user32 kernel32)

set_target_properties(AltShiftMenuFix PROPERTIES
    CXX_EXTENSIONS OFF
    PREFIX ""
    OUTPUT_NAME "AltShiftMenuFix")

if(MSVC)
    target_compile_options(AltShiftMenuFix PRIVATE /W4 /WX /permissive- /sdl /guard:cf /utf-8)
    target_link_options(AltShiftMenuFix PRIVATE /guard:cf /DYNAMICBASE /NXCOMPAT /HIGHENTROPYVA /CETCOMPAT)
else()
    target_compile_options(AltShiftMenuFix PRIVATE -Wall -Wextra -Wpedantic -Werror -fstack-protector-strong)
    target_link_options(AltShiftMenuFix PRIVATE
        -static-libgcc
        -static-libstdc++
        -Wl,--dynamicbase
        -Wl,--nxcompat
        -Wl,--high-entropy-va)
endif()

if(BUILD_TESTING)
    add_executable(AltShiftMenuFixIntegrationTest
        tests/IntegrationTest.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/generated/TestHost.rc)
    target_include_directories(AltShiftMenuFixIntegrationTest PRIVATE include)
    target_compile_features(AltShiftMenuFixIntegrationTest PRIVATE cxx_std_17)
    target_compile_definitions(AltShiftMenuFixIntegrationTest PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0601
        WINVER=0x0601)
    target_link_libraries(AltShiftMenuFixIntegrationTest PRIVATE user32 kernel32)

    set_target_properties(AltShiftMenuFixIntegrationTest PROPERTIES CXX_EXTENSIONS OFF)

    if(MSVC)
        target_compile_options(AltShiftMenuFixIntegrationTest PRIVATE /W4 /WX /permissive- /sdl /guard:cf /utf-8)
        target_link_options(AltShiftMenuFixIntegrationTest PRIVATE /guard:cf /DYNAMICBASE /NXCOMPAT /HIGHENTROPYVA /CETCOMPAT)
    else()
        target_compile_options(AltShiftMenuFixIntegrationTest PRIVATE -Wall -Wextra -Wpedantic -Werror -fstack-protector-strong)
        target_link_options(AltShiftMenuFixIntegrationTest PRIVATE -static-libgcc -static-libstdc++)
    endif()

    add_test(
        NAME AltShiftMenuFix.Integration
        COMMAND AltShiftMenuFixIntegrationTest $<TARGET_FILE:AltShiftMenuFix>)
endif()

install(TARGETS AltShiftMenuFix RUNTIME DESTINATION .)

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_NAME NppAltShiftMenuFix)
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "NppAltShiftMenuFix-${PROJECT_VERSION}-x64")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
