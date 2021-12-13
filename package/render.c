//
// Created by human on 12/13/21.
//

#include "render.h"
#include "types.h"


#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>



TTF_Font *font;
RenderData *rd;
MoveHooks mh;

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int rv = IMG_Init(IMG_INIT_PNG);
    if ((rv & IMG_INIT_PNG) != IMG_INIT_PNG) {
        fprintf(stderr, "Error initializing IMG: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "Error initializing TTF: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

void moveTo(int x, int y) {
    // Prevent falling off the grid
    if (x < 0 || x >= GRIDSIZE || y < 0 || y >= GRIDSIZE)
        return;

    // Sanity check: player can only move to 4 adjacent squares
    if (!(abs(rd->thisPlayer.p.x - x) == 1 && abs(rd->thisPlayer.p.y - y) == 0) &&
        !(abs(rd->thisPlayer.p.x - x) == 0 && abs(rd->thisPlayer.p.y - y) == 1)) {
        fprintf(stderr, "Invalid move attempted from (%d, %d) to (%d, %d)\n", rd->thisPlayer.p.x, rd->thisPlayer.p.y, x,
                y);
        return;
    }

    rd->thisPlayer.p.x = x;
    rd->thisPlayer.p.y = y;

    if (rd->grid[x][y] == TILE_TOMATO) {
        rd->grid[x][y] = TILE_GRASS;
//        rd->score++;
//        rd->numTomatoes--;
//        if (numTomatoes == 0) {
//            level++;
//            initGrid();
//        }
    }
}

void handleKeyDown(SDL_KeyboardEvent *event) {
    // ignore repeat events if key is held down
    if (event->repeat)
        return;

    if (event->keysym.scancode == SDL_SCANCODE_Q || event->keysym.scancode == SDL_SCANCODE_ESCAPE)
        mh.end_game();

    if (event->keysym.scancode == SDL_SCANCODE_UP || event->keysym.scancode == SDL_SCANCODE_W) {
        mh.move_top();
        moveTo(rd->thisPlayer.p.x, rd->thisPlayer.p.y - 1);
    }

    if (event->keysym.scancode == SDL_SCANCODE_DOWN || event->keysym.scancode == SDL_SCANCODE_S) {
        mh.move_bottom();
        moveTo(rd->thisPlayer.p.x, rd->thisPlayer.p.y + 1);
    }

    if (event->keysym.scancode == SDL_SCANCODE_LEFT || event->keysym.scancode == SDL_SCANCODE_A) {
        mh.move_left();
        moveTo(rd->thisPlayer.p.x - 1, rd->thisPlayer.p.y);
    }

    if (event->keysym.scancode == SDL_SCANCODE_RIGHT || event->keysym.scancode == SDL_SCANCODE_D) {
        mh.move_right();
        moveTo(rd->thisPlayer.p.x + 1, rd->thisPlayer.p.y);
    }
}

void processInputs() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                mh.end_game();
                break;

            case SDL_KEYDOWN:
                handleKeyDown(&event.key);
                break;

            default:
                break;
        }
    }
}

void drawGrid(SDL_Renderer *renderer, SDL_Texture *grassTexture, SDL_Texture *tomatoTexture, SDL_Texture *playerTexture) {

    // draw grid
    SDL_Rect dest;
    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            dest.x = 64 * i;
            dest.y = 64 * j + HEADER_HEIGHT;
            SDL_Texture *texture = (rd->grid[i][j] == TILE_GRASS) ? grassTexture : tomatoTexture;
            SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
            SDL_RenderCopy(renderer, texture, NULL, &dest);
        }
    }

    // draw thisPlayer
    dest.x = 64 * rd->thisPlayer.p.x;
    dest.y = 64 * rd->thisPlayer.p.y + HEADER_HEIGHT;
    SDL_QueryTexture(playerTexture, NULL, NULL, &dest.w, &dest.h);
    SDL_RenderCopy(renderer, playerTexture, NULL, &dest);

    // draw otherPlayers

    PlayerStat playerPos;
    for (int i = 1; i < rd->gd.number_of_active_players; ++i) {
        playerPos = rd->other_player[i];
        if (playerPos.id == rd->thisPlayer.id)
            continue;

        dest.x = 64 * playerPos.p.x;
        dest.y = 64 * playerPos.p.y + HEADER_HEIGHT;
        SDL_QueryTexture(playerTexture, NULL, NULL, &dest.w, &dest.h);
        SDL_RenderCopy(renderer, playerTexture, NULL, &dest);

    }


}

void drawUI(SDL_Renderer *renderer) {
    // largest score/level supported is 2147483647
    char scoreStr[18];
    char levelStr[18];
    sprintf(scoreStr, "Score: %d", rd->gd.current_score);
    sprintf(levelStr, "Level: %d", rd->gd.current_level);

    SDL_Color white = {255, 255, 255};
    SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreStr, white);
    SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);

    SDL_Surface *levelSurface = TTF_RenderText_Solid(font, levelStr, white);
    SDL_Texture *levelTexture = SDL_CreateTextureFromSurface(renderer, levelSurface);

    SDL_Rect scoreDest;
    TTF_SizeText(font, scoreStr, &scoreDest.w, &scoreDest.h);
    scoreDest.x = 0;
    scoreDest.y = 0;

    SDL_Rect levelDest;
    TTF_SizeText(font, levelStr, &levelDest.w, &levelDest.h);
    levelDest.x = GRID_DRAW_WIDTH - levelDest.w;
    levelDest.y = 0;

    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreDest);
    SDL_RenderCopy(renderer, levelTexture, NULL, &levelDest);

    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    SDL_FreeSurface(levelSurface);
    SDL_DestroyTexture(levelTexture);
}

void render(RenderData *render_data, MoveHooks moveHooks) {
    mh = moveHooks;
    rd = render_data;

    initSDL();

    font = TTF_OpenFont("../resources/Burbank-Big-Condensed-Bold-Font.otf", HEADER_HEIGHT);
    if (font == NULL) {
        fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

//    rd->thisPlayer.x = rd->thisPlayer.y = GRIDSIZE / 2;
//    initGrid();

    SDL_Window *window = SDL_CreateWindow("Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                          WINDOW_HEIGHT, 0);

    if (window == NULL) {
        fprintf(stderr, "Error creating app window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL) {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Texture *grassTexture = IMG_LoadTexture(renderer, "../resources/grass.png");
    SDL_Texture *tomatoTexture = IMG_LoadTexture(renderer, "../resources/tomato.png");
    SDL_Texture *playerTexture = IMG_LoadTexture(renderer, "../resources/player.png");

    // main game loop
    while (!rd->shouldExit) {
        SDL_SetRenderDrawColor(renderer, 0, 105, 6, 255);
        SDL_RenderClear(renderer);

        processInputs();

        drawGrid(renderer, grassTexture, tomatoTexture, playerTexture);
        drawUI(renderer);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // 16 ms delay to limit display to 60 fps
    }

    // clean up everything
    SDL_DestroyTexture(grassTexture);
    SDL_DestroyTexture(tomatoTexture);
    SDL_DestroyTexture(playerTexture);

    TTF_CloseFont(font);
    TTF_Quit();

    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
