cmake_minimum_required(VERSION 3.20)
project(server C)
project(client C)

set(CMAKE_C_STANDARD 99)


add_executable(server server.c shared.h)
add_executable(client cli.c shared.h)
