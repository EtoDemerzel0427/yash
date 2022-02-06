cmake_minimum_required(VERSION 3.21)
project(yash C)

set(CMAKE_C_STANDARD 99)
include_directories(/usr/include/readline)
add_executable(yash main.c parse.c parse.h job.c job.h)
target_link_libraries(yash readline)
add_compile_definitions(DEBUG)