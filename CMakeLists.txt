cmake_minimum_required(VERSION 3.21.3)
project(chess)

include_directories(include)
file(GLOB SOURCES "src/*.c")
add_executable(chess.out ${SOURCES})
