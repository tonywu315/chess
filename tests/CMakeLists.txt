project(tests)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Create test
add_executable(test.out test.c)
add_test(NAME perft1 COMMAND test.out 1)
add_test(NAME perft2 COMMAND test.out 2)
add_test(NAME perft3 COMMAND test.out 3)
add_test(NAME perft4 COMMAND test.out 4)
add_test(NAME perft5 COMMAND test.out 5)
add_test(NAME perft6 COMMAND test.out 6)

# Include library and headers
target_link_libraries(test.out chesslib)
target_include_directories(test.out PRIVATE ${CMAKE_SOURCE_DIR}/include)
