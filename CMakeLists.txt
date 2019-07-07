cmake_minimum_required(VERSION 3.0.2)
project(c-util)

include_directories("${PROJECT_SOURCE_DIR}/")

add_executable(test joystick_ps3.c test/test.c)
target_link_libraries(test pthread)
