//
// Created by human on 12/7/21.
//

#ifndef CLIENT_TYPES_H
#define CLIENT_TYPES_H
#include "pthread.h"

#define FROM_SERVER -1

typedef enum{
    YOUR_ID,
    PLAYER_DISCONNECTED,
    PLAYER_JOINED, // todo

    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_FORWARD,
    MOVE_BACKWARD,

    SCORE_UPDATE,
    LEVEL_UPDATE
}  Code;

typedef enum
{
    TILE_GRASS,
    TILE_TOMATO
} TILETYPE;

typedef struct {
    int from_player;
    Code code;
    int data;
} Message;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position p;
    int id;
    int is_active;
} PlayerStat;

typedef struct {
    int number_of_active_players;
    int current_level;
    int current_score;
    int number_of_tomatoes;
} GameData;



typedef struct {

    int player_number;
    int sock;
    int is_active;
    pthread_mutex_t  recv_thread;

} Player;
extern const char* magic_cl;
#endif //CLIENT_TYPES_H
