cmake_minimum_required(VERSION 3.20)
project(server C)
project(client C)
project(game C)

set(CMAKE_C_STANDARD 99)

link_libraries( pthread SDL2 SDL2_image SDL2_ttf)

add_executable(game client.c)

add_executable(server
        server.h server.c
        package/shared.h package/shared.c
        package/types.h package/types.c
        )

add_executable(client
        cli.c
        package/shared.h package/shared.c
        package/types.h package/types.c
        package/render.c package/render.h
        )

