cmake_minimum_required(VERSION 3.10)

project(ZTD_puzzle_solver)

add_executable(main main.c)

add_executable(test test.c)

add_custom_command(
    TARGET test
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/test files" "$<TARGET_FILE_DIR:test>/test files"
)
