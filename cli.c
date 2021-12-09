// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include "package/shared.h"
#include "package/types.h"


typedef struct {
    int server_sock;
    int my_id;
    GameData gd;
    PlayerStat *stats;
} ClientData;

inline void connection_job(int, char const *[]);

inline void init();

inline void cleanup();

inline void do_initialize_client();

ClientData *cd;

int main(int argc, char const *argv[]) {
    init();
    connection_job(argc, argv);
    cleanup();
    return 0;
}

void init() {
    cd = malloc(sizeof(ClientData));
}

void cleanup() {
    free(cd->stats);
    free(cd);
}

void connection_job(int argc, char const *argv[]) {

    int PORT = 8081; // todo read from argv
    int sock = 0;

    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(-1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(-1);
    }

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        exit(-1);
    }
    send(sock, magic_cl, strlen(magic_cl), 0);
    printf("magic message sent and client is now connected\n");
    cd->server_sock = sock;
    do_initialize_client();
}

inline void recv_game_data();

inline void recv_self_id();

inline void recv_player_stats();

inline void post_init();

void do_initialize_client() {
    recv_game_data();
    recv_self_id();
    post_init();
    recv_player_stats();
    printf("my_id: %d \n",cd->my_id);
    printf("game_data: { #ofPlayers:%d  current_score:%d current_level:%d } \n",
           cd->gd.number_of_active_players,
           cd->gd.current_score,
           cd->gd.current_level
           );

    printf("Players stats: \n");
    for (int i = 0; i < cd->gd.number_of_active_players; ++i) {
        printf("Player %d : stats:{ id:%d, Position:{%d,%d} is_active:%d}\n",
               i,
               cd->stats[i].id,
               cd->stats[i].p.x,
               cd->stats[i].p.y,
               cd->stats[i].is_active
               );
    }

}

void recv_game_data() {
    int i = recv(cd->server_sock, &cd->gd, sizeof(GameData), 0);
    if (i != sizeof(GameData)) {
        printf("GameData Not Received");
        exit(-1);
    }
}

void recv_self_id() {
    Message m;
    long int i = recv(cd->server_sock, &m, sizeof(Message), 0);
    if (i != sizeof(Message) || m.code != YOUR_ID || m.from_player != FROM_SERVER) {
        printf("received a message from server (or not ?) which is not my id, will terminate");
        exit(-1);
    }
    cd->my_id = m.data;
}


void post_init() {
    cd->stats = calloc(cd->gd.number_of_active_players, sizeof(PlayerStat));
}

void recv_player_stats() {
    long int i = recv(
            cd->server_sock,
            cd->stats,
            sizeof(PlayerStat) * cd->gd.number_of_active_players,
            0
    );
    if(i != sizeof(PlayerStat) * cd->gd.number_of_active_players){
        printf("could not receive player stats");
        exit(-1);
    }
}