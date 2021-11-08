project(tests)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Create test
add_executable(test.out test.c)
add_test(NAME test COMMAND test.out)

# Include library and headers
target_link_libraries(test.out chesslib)
target_include_directories(test.out PRIVATE ${CMAKE_SOURCE_DIR}/include)
