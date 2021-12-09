//
// Created by human on 12/7/21.
//

#ifndef CLIENT_TYPES_H
#define CLIENT_TYPES_H
#include "pthread.h"
#define FROM_SERVER -1
#define GRID_SIZE 640
typedef struct {
    int from_player;
    int code;
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
} GameData;

enum code{
    YOUR_ID,
    PLAYER_DISCONNECTED,

    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_FORWARD,
    MOVE_BACKWARD,

    SCORE_UPDATE,
    LEVEL_UPDATE
};

typedef struct {

    int player_number;
    int sock;
    int is_active;
    pthread_mutex_t  recv_thread;

} Player;
extern const char* magic_cl;
#endif //CLIENT_TYPES_H
