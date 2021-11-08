project(tests)

add_executable(test1.out test.c)
add_test(NAME test1 COMMAND test1.out)
target_link_libraries(test1.out chesslib)
target_include_directories(test1.out PRIVATE ${CMAKE_SOURCE_DIR}/include)
