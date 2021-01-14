cmake_minimum_required(VERSION 3.0.2)
project(c-util)

set(CMAKE_C_STANDARD 99)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(joystick_test src/joystick.c test/test.c)
target_link_libraries(joystick_test pthread)
