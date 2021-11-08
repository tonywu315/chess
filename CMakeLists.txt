cmake_minimum_required(VERSION 3.21)
project(chess)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Header files and C files
file(GLOB HEADERS "include/*.h")
set(SOURCES
    src/benchmark.c
    src/board.c
    src/evaluation.c
    src/move_generation.c
    src/move.c
    src/search.c
)

# Executable file
add_executable(chess.out ${SOURCES} src/main.c)
target_include_directories(chess.out PRIVATE include)

# Chess library file for tests
add_library(chesslib ${SOURCES})
target_include_directories(chesslib PRIVATE include)

# Tests
include(CTest)
enable_testing()
add_subdirectory(tests)
