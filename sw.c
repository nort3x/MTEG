//
// Created by human on 12/4/21.
//

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include "shared.h"


typedef struct {

    int player_number;
    int sock;
    pthread_t receiver_thread;
    int is_active;

} Player;


typedef struct {
    pthread_mutex_t game_data;
    pthread_mutex_t server_internals;

} Locks;

typedef struct {
    int sv_port;
    Player *players;
    int accept_socket;
} ServerInternals;


inline Player add_player(Player p);

inline void remove_player(Player p);

inline void send_current_state_to_player(Player p);

inline void send_to_other_players(Player sender, Message message);

inline void socket_creation_and_accept_job();

inline void init(int argc, char const *argv[]);

inline int confirm_is_client(int sock);

inline void accept_player(int socket);

inline void send_player_its_id(Player p);

inline void send_message(Player p, Message m);

inline void register_receiver_thread_for_player(Player p);

inline void *receiver_job(void *player);

inline void cleanup();

GameData *gd;
ServerInternals *sv;
Locks *locks;

int main(int argc, char const *argv[]) {

    init(argc, argv);
    socket_creation_and_accept_job();
    cleanup();

    return 0;
}


void init(int argc, char const *argv[]) {
    gd = malloc(sizeof(GameData));
    gd->current_level = 1;
    gd->current_score = 0;
    gd->number_of_active_players = 0;

    sv = malloc(sizeof(ServerInternals));
    sv->players = calloc(gd->number_of_active_players, sizeof(Player));
    sv->sv_port = 8081; //todo read this from argv


    locks = malloc(sizeof(Locks));
    pthread_mutex_t l1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t l2 = PTHREAD_MUTEX_INITIALIZER;

    locks->game_data = l1;
    locks->server_internals = l2;

}

void cleanup() {
    free(sv->players);
    free(gd);
}

// critical section so we lock
void remove_player(Player p) {
    pthread_mutex_lock(&locks->game_data);
    pthread_mutex_lock(&locks->server_internals);

    sv->players[p.player_number].is_active = 0;
    pthread_cancel(sv->players[p.player_number].receiver_thread);
    sv->players[p.player_number].player_number = -1; // effectively disabling player
    gd->number_of_active_players--;

    pthread_mutex_unlock(&locks->server_internals);
    pthread_mutex_unlock(&locks->game_data);


}

Player add_player(Player p) {

    p.player_number = gd->number_of_active_players++;
    p.player_number = gd->total_number_of_players++;
    sv->players = realloc(sv->players, sizeof(Player) * (gd->number_of_active_players + 1));
    sv->players[p.player_number] = p;

    return p;
}


void send_current_state_to_player(Player p) {
    send(p.sock, gd, sizeof(GameData), 0);
}

void send_to_other_players(Player sender, Message message) {
    message.from_player = sender.player_number;
    for (int i = 0; i < gd->total_number_of_players; ++i) {
        Player objective = sv->players[i];
        if (objective.player_number != -1 && // not inactive
            sender.player_number != objective.player_number) // not self
            send_message(objective, message);
    }
}

void socket_creation_and_accept_job() {

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(sv->sv_port);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *) &address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
                                 (socklen_t *) &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        if (!confirm_is_client(new_socket)) {
            printf("bad connection , client disconnected or wasn't client at all\n");
            close(new_socket);
            continue;
        }

        accept_player(new_socket);

    }

}


int confirm_is_client(int sock) {
    char buffer[1024] = {0};
    int long read_size = read(sock, buffer, 1024);

    if (read_size != strlen(magic_cl))
        return 0;
    return !strncmp(magic_cl, buffer, strlen(magic_cl));
}

void accept_player(int new_socket) {
    Player p = {0, new_socket};

    // here we are dealing with  a critical section
    // client needs to know its id and game_state
    // we can't do it concurrently

    pthread_mutex_lock(&locks->game_data);
    pthread_mutex_lock(&locks->server_internals);

    p = add_player(p);
    send_current_state_to_player(p);

    pthread_mutex_unlock(&locks->server_internals);
    pthread_mutex_unlock(&locks->game_data);

    send_player_its_id(p);

    register_receiver_thread_for_player(p);


}

void send_player_its_id(Player p) {
    send_message(p, (Message) {FROM_SERVER, YOUR_ID, p.player_number});
}

void send_message(Player p, Message m) {
    int long result = send(p.sock, &m, sizeof(Message), 0);
    if (result != sizeof(Message)) {
        // if anything goes wrong we say bye bye to player , our game our rules
        pthread_mutex_lock(&locks->game_data);
        pthread_mutex_lock(&locks->server_internals);

        remove_player(p);

        pthread_mutex_unlock(&locks->server_internals);
        pthread_mutex_unlock(&locks->game_data);

    }
}


// here we register one thread for each player which will always listen to process incoming message
void register_receiver_thread_for_player(Player p) {

    pthread_mutex_lock(&locks->game_data);
    pthread_mutex_lock(&locks->server_internals);

    pthread_create(
            &sv->players[p.player_number].receiver_thread,
            NULL,
            receiver_job,
            &sv->players[p.player_number]
    );


    pthread_mutex_unlock(&locks->server_internals);
    pthread_mutex_unlock(&locks->game_data);
}

Message *read_message_from_player(Player *p) {
    Message *m = malloc(sizeof(Message));
    int long res = recv(p->sock, m, sizeof(Message), 0);
    return res == -1 ? NULL : m;
}

inline void process_msg(Player *objective, Message *m);

void player_left_game(Player *objective) {
    send_to_other_players(
            *objective,
            (Message) {
                    objective->player_number,
                    PLAYER_DISCONNECTED,
                    1
            }
    );
}

void *receiver_job(void *_player) {

    Player *player = (Player *) _player;
    while (player->is_active) {
        Message *msg = read_message_from_player(player);
        if (msg == NULL) {

            player_left_game(player);
            remove_player(*player);

        }
        process_msg(player, msg);
        free(msg);
    }

    return NULL;
}

void player_moved(Player *objective, Message *msg) {
    send_to_other_players(*objective, *msg);
}


void process_msg(Player *objective, Message *msg) {
    printf("received %d %d", msg->code, msg->data);

    switch (msg->code) {

        case MOVE_BACKWARD:
        case MOVE_FORWARD:
        case MOVE_LEFT:
        case MOVE_RIGHT:
            player_moved(objective, msg);
            break;
        default:
            player_left_game(objective);
            break;
    }
}