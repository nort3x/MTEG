cmake_minimum_required(VERSION 3.20)
project(server C)
project(client C)

set(CMAKE_C_STANDARD 99)

link_libraries( pthread SDL2 SDL2_image SDL2_ttf)


add_executable(
        server.h server.c
        shared.h shared.c
        types.h types.c
        )

add_executable(

        client.c
        shared.h shared.c
        types.h types.c
        render.c render.h
        )

