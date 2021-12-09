//
// Created by human on 12/8/21.
//

#ifndef TEST_SERVER_H
#define TEST_SERVER_H
typedef struct {
    pthread_mutex_t game_data;
    pthread_mutex_t server_internals;

} Locks;


typedef struct {
    int server_fd;
    int new_socket;
    struct sockaddr_in address;
    int opt ;
    int address_len;

} ServerSocketInfo;

typedef struct {
    int sv_port;
    Player *players;
    PlayerStat * stat;
    ServerSocketInfo serverSocketInfo;
} ServerInternals;

#endif //TEST_SERVER_H
