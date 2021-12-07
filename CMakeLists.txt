cmake_minimum_required(VERSION 3.20)
project(server C)
project(client C)
project(test C)
set(CMAKE_C_STANDARD 99)

link_libraries( pthread)
add_executable(server sw.c shared.h)
add_executable(client cli.c shared.h)

add_executable(test test.c package/shared.c package/shared.h package/types.h)
