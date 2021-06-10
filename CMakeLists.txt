cmake_minimum_required(VERSION 3.0.2)
project(c-util)

set(CMAKE_C_STANDARD 99)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(joystick_test src/joystick.c src/joystick_map.c test/test.c)
target_link_libraries(joystick_test pthread)

set_target_properties(joystick_test PROPERTIES C_STANDARD 99)

target_compile_options(joystick_test PRIVATE -Wall -Wextra -Werror -pedantic-errors -Wconversion -Wsign-conversion  -Wimplicit-function-declaration)
