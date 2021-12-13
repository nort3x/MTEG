//
// Created by human on 12/13/21.
//

#ifndef CLIENT_RENDER_H
#define CLIENT_RENDER_H

// Dimensions for the drawn grid (should be GRIDSIZE * texture dimensions)
#define GRID_DRAW_WIDTH 640
#define GRID_DRAW_HEIGHT 640

#define WINDOW_WIDTH GRID_DRAW_WIDTH
#define WINDOW_HEIGHT (HEADER_HEIGHT + GRID_DRAW_HEIGHT)

// Header displays current score
#define HEADER_HEIGHT 50

// Number of cells vertically/horizontally in the grid
#define GRIDSIZE 10


#include <stdbool.h>
#include "types.h"




typedef struct {
    GameData  gd;
    PlayerStat thisPlayer;
    bool shouldExit;
    TILETYPE (*grid)[GRIDSIZE];
    PlayerStat* other_player;
} RenderData;

typedef struct {
    void (*move_left)(void);
    void (*move_right)(void);
    void (*move_top)(void);
    void (*move_bottom)(void);
    void (*end_game)(void);
}MoveHooks;

void render(RenderData *render_data,MoveHooks moveHooks);
#endif //CLIENT_RENDER_H
