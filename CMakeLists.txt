cmake_minimum_required(VERSION 3.16.3)

project(chess C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(CMAKE_C_FLAGS " -pthread -O3 -march=native")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

find_package(Threads REQUIRED)

# C files
set(SOURCES
    src/attacks.c
    src/benchmark.c
    src/board.c
    src/evaluation.c
    src/move_generation.c
    src/move_order.c
    src/move.c
    src/quiescence.c
    src/search.c
    src/transposition.c
    src/uci.c
)

# Create executable file
add_executable(chess.out ${SOURCES} src/main.c)
target_include_directories(chess.out PRIVATE include)
target_link_libraries(chess.out PRIVATE m)

# Add preprocessor macros
target_compile_definitions(chess.out PRIVATE DEBUG)

# Chess library file for tests
add_library(chesslib ${SOURCES})
target_include_directories(chesslib PRIVATE include)

# Tests
include(CTest)
enable_testing()
add_subdirectory(tests)
