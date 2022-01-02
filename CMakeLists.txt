cmake_minimum_required(VERSION 3.21)

project(chess C)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(CMAKE_C_FLAGS "-O3 -march=native")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

# C files
set(SOURCES
    src/attacks.c
    src/benchmark.c
    src/bitboard.c
    src/evaluation.c
    src/game.c
    src/move_generation.c
    src/move.c
    src/search.c
)

# Create executable file
add_executable(chess.out ${SOURCES} src/main.c)
target_include_directories(chess.out PRIVATE include)
target_link_libraries(chess.out Threads::Threads)

# Chess library file for tests
add_library(chesslib ${SOURCES})
target_include_directories(chesslib PRIVATE include)

# Tests
include(CTest)
enable_testing()
add_subdirectory(tests)
