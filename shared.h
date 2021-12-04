//
// Created by human on 12/4/21.
//

#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#define FROM_SERVER -1

typedef struct {
    int from_player;
    int code;
    int data;
} Message;

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
    MOVE_BACKWARD

};

char *magic_cl = "Yes I Am Your Client!";
#endif //CLIENT_SERVER_H
