CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)

project(jagle)
cmake_policy(SET CMP0135 NEW)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.3.0
)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 9.1.0
)
FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG        v2.2.0
)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.3.2
)

FetchContent_MakeAvailable(tomlplusplus fmt cli11 Catch2)

# ANTLR4
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/antlr4/runtime/Cpp/cmake)
add_definitions(-DANTLR4CPP_STATIC)
#set(ANTLR4_WITH_STATIC_CRT OFF)
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/antlr-4.12.0-complete.jar)
include(ExternalAntlr4Cpp)
include_directories(${ANTLR4_INCLUDE_DIRS})

find_package(ANTLR REQUIRED)

# Jagle lexer, parser, and visitor generations
antlr_target(JagleGrammar Jagle.g4 PACKAGE jagle LEXER PARSER VISITOR)
message(STATUS "Generated grammar: ${ANTLR_JagleGrammar_OUTPUT_DIR}")

# jagle.exe
add_executable(jagle main.cpp visitor.cpp)

target_sources(jagle PRIVATE ${ANTLR_JagleGrammar_CXX_OUTPUTS})

target_link_libraries(jagle PRIVATE antlr4_static fmt::fmt-header-only)

target_include_directories(jagle PRIVATE ${ANTLR_JagleGrammar_OUTPUT_DIR})
target_include_directories(jagle PRIVATE ${tomlplusplus_SOURCE_DIR})
target_include_directories(jagle PRIVATE ${cli11_SOURCE_DIR}/include)

target_compile_features(jagle PRIVATE cxx_std_17)

# jagle_tests.exe
add_executable(jagle_tests test.cpp visitor.cpp)

target_sources(jagle_tests PRIVATE ${ANTLR_JagleGrammar_CXX_OUTPUTS})

target_link_libraries(jagle_tests PRIVATE Catch2::Catch2WithMain antlr4_static fmt::fmt-header-only)

target_include_directories(jagle_tests PRIVATE ${ANTLR_JagleGrammar_OUTPUT_DIR})

target_compile_features(jagle_tests PRIVATE cxx_std_17)

message(STATUS "Catch2 extras: ${Catch2_SOURCE_DIR}/extras")
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(jagle_tests)
