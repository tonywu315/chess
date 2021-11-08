project(tests)

# Create test
add_executable(test.out test.c)
add_test(NAME test COMMAND test.out)

# Include library and headers
target_link_libraries(test.out chesslib)
target_include_directories(test.out PRIVATE ${CMAKE_SOURCE_DIR}/include)
