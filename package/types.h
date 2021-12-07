//
// Created by human on 12/7/21.
//

#ifndef CLIENT_TYPES_H
#define CLIENT_TYPES_H
#include "pthread.h"
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
    int total_number_of_players;
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

    POSITION_UPDATE
};

typedef struct {

    int player_number;
    int sock;
    int is_active;
    pthread_mutex_t  pthreadMutex;

} Player;

//char *magic_cl = "Yes I Am Your Client!";
#endif //CLIENT_TYPES_H
