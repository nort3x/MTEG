//
// Created by human on 12/8/21.
//

#include <malloc.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "package/types.h"
#include "package/shared.h"
#include "server.h"
#include "package/render.h"

// will allocate and extract basic information for runtime
inline void init(int argc, char const *argv[]);

inline void cleanup();

inline void init_grid();

// main routine which consist of accepting clients and managing them
// it will be given to main thread
inline void main_routine();


int main(int argc, char const *argv[]) {

    init(argc, argv);
    main_routine();
    cleanup();

    return 0;
}

GameData *gd = NULL;
ServerInternals *sv = NULL;
Locks *locks = NULL;
TILETYPE ** grid = NULL;

void init(int argc, char const *argv[]) {
    gd = malloc(sizeof(GameData));
    gd->current_level = 1;
    gd->current_score = 0;
    gd->number_of_active_players = 0;
    init_grid();

    sv = malloc(sizeof(ServerInternals));
    sv->players = calloc(gd->number_of_active_players, sizeof(Player));
    sv->sv_port = 8081; //todo read this from argv
    sv->stat = calloc(gd->number_of_active_players, sizeof(PlayerStat));

    locks = malloc(sizeof(Locks));
    pthread_mutex_t l1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t l2 = PTHREAD_MUTEX_INITIALIZER;

    locks->game_data = l1;
    locks->server_internals = l2;
}

void cleanup() {
    free(sv->players);
    free(gd);
    free(locks);
    free(grid);
}


/*----------------main routine----------------*/
inline void create_server_socket();

inline int confirm_is_client(int socket);

inline void accept_player(int socket);

void main_routine() {

    create_server_socket();

    int new_socket;
    // infinite loop for accepting new connections , initialize them and put them for management
    while (1) {
        // accept socket
        if ((new_socket = accept(sv->serverSocketInfo.server_fd, (struct sockaddr *) &(sv->serverSocketInfo.address),
                                 (socklen_t *) &sv->serverSocketInfo.address_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // confirm it's our
        if (!confirm_is_client(new_socket)) {
            printf("bad connection , client disconnected or wasn't client at all\n");
            close(new_socket);
            continue;
        }

        //give initialize them
        accept_player(new_socket);

    }

}

void create_server_socket() {
    sv->serverSocketInfo.opt = 1;
    sv->serverSocketInfo.address_len = sizeof(sv->serverSocketInfo.address);

    // Creating socket file descriptor
    if ((sv->serverSocketInfo.server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sv->serverSocketInfo.server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &(sv->serverSocketInfo.opt), sizeof(sv->serverSocketInfo.opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    sv->serverSocketInfo.address.sin_family = AF_INET;
    sv->serverSocketInfo.address.sin_addr.s_addr = INADDR_ANY;
    sv->serverSocketInfo.address.sin_port = htons(sv->sv_port);

    if (bind(sv->serverSocketInfo.server_fd, (struct sockaddr *) &sv->serverSocketInfo.address,
             sizeof(sv->serverSocketInfo.address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(sv->serverSocketInfo.server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

/*-------------confirm is client and accept it--------------*/

int confirm_is_client(int socket) {
    char buff[1024];
    if (recv(socket, buff, strlen(magic_cl), 0) != strlen(magic_cl)) return 0;
    if (strncmp(magic_cl, buff, strlen(magic_cl)) == 0) return 1;
    return 0;
}

/*------init client----*/
inline int send_game_data(Player *p);

inline int send_id(Player *p);

inline Player *add_player(Player p);

inline void register_receiver_thread_for_player(Player *p);

inline void make_state_for_player(Player *p);

inline int send_stats(Player *p);
inline int send_grid(Player* p);

inline void remove_player(Player *p);

inline void *receiver_job(void *ptr);

void accept_player(int socket) {
    int success = 1;
    Player p = {0, socket};
    pthread_mutex_lock(&locks->game_data);
    pthread_mutex_lock(&locks->server_internals);

    Player *pp = add_player(p);
    make_state_for_player(pp);


    success &= send_game_data(pp);

    if (success)
        success &= send_id(pp);

    if (success)
        success &= send_stats(pp);
//    if(success)
//        success &= send_grid(pp);

    if (success) {
        pp->is_active = 1;
        register_receiver_thread_for_player(pp);
    }
    else
        remove_player(pp);

    pthread_mutex_unlock(&locks->server_internals);
    pthread_mutex_unlock(&locks->game_data);

}


Player *add_player(Player p) {
    p.player_number = gd->number_of_active_players + 1;
    sv->players = add_player_to_list(p, sv->players, gd->number_of_active_players);
    gd->number_of_active_players++;
    return &(sv->players[gd->number_of_active_players - 1]);
}

int get_index_of_player(Player p) {
    int p_pos = -1;
    for (int i = 0; i < gd->number_of_active_players; ++i) {
        if (sv->players[i].player_number == p.player_number) {
            p_pos = i;
            break;
        }
    }
    return p_pos;
}

int get_index_of_player_stat(Player p) {
    int p_pos = -1;
    for (int i = 0; i < gd->number_of_active_players; ++i) {
        if (sv->stat[i].id == p.player_number) {
            p_pos = i;
            break;
        }
    }
    return p_pos;
}

void remove_player(Player *p) {
    int p_pos = get_index_of_player(*p);
    int ps_pos = get_index_of_player_stat(*p);
    if (p_pos >= 0 && ps_pos >= 0) {
        sv->players = remove_player_from_list(p_pos, sv->players, gd->number_of_active_players);
        sv->stat = remove_player_stat_from_list(ps_pos, sv->stat, gd->number_of_active_players);
    } else {
        printf("bad things are happening");
        exit(-1);
    }
    p->is_active = 0;
}

int send_game_data(Player *p) {
    long int i = send(p->sock, gd, sizeof(GameData), 0);
    return i == sizeof(GameData) ? 1 : 0;
}

int send_id(Player *p) {
    Message m = {FROM_SERVER, YOUR_ID, p->player_number};
    return send(p->sock, &m, sizeof(Message), 0) == sizeof(GameData) ? 1 : 0;
}

void register_receiver_thread_for_player(Player *p) {
    pthread_create(
            (void *) &p->recv_thread,
            NULL,
            receiver_job,
            p
    );
}

Position random_position() {
    return (Position) {
            (int) (drand48() * GRID_SIZE),
            (int) (drand48() * GRID_SIZE)
    };
}

void make_state_for_player(Player *p) {
    PlayerStat ps = {random_position(), p->player_number, 1};
    sv->stat = add_player_stat_to_list(ps, sv->stat, gd->number_of_active_players-1);
}

int send_stats(Player *p) {
    size_t size_stat = sizeof(PlayerStat) * gd->number_of_active_players;
    return send(p->sock, sv->stat, size_stat, 0)
           == size_stat ? 1 : 0;
}

int send_grid(Player* p){
    size_t size_grid = sizeof(TILETYPE)*GRIDSIZE*GRIDSIZE;
    return send(p->sock, grid, size_grid, 0)
           == size_grid ? 1 : 0;
}


/*-------------game logics--------------*/
inline void handle_message(Player *p, Message m);

inline void player_moved(Player *p, Message m);

inline void player_removed(Player *p);

void *receiver_job(void *ptr) {
    Player *p = ptr;

    Message m;
    while (p->is_active) {
        if (recv(p->sock, &m, sizeof(Message), 0) != sizeof(Message))
            break;
        handle_message(p, m);
    }

    pthread_mutex_lock(&locks->game_data);
    pthread_mutex_lock(&locks->server_internals);

    remove_player(p);

    pthread_mutex_unlock(&locks->game_data);
    pthread_mutex_unlock(&locks->server_internals);

    printf("player: %d removed, either disconnected or misbehaved\n", p->player_number);
    return NULL;
}

void handle_message(Player *p, Message m) {
    switch (m.code) {
        case MOVE_RIGHT:
        case MOVE_LEFT:
        case MOVE_FORWARD:
        case MOVE_BACKWARD:
            player_moved(p, m);
            break;

        default:
            player_removed(p);
            pthread_mutex_lock(&locks->game_data);
            pthread_mutex_lock(&locks->server_internals);

            remove_player(p);

            pthread_mutex_unlock(&locks->game_data);
            pthread_mutex_unlock(&locks->server_internals);
            break;


    }
}

void send_message(Player* p, Message m) {
    int long result = send(p->sock, &m, sizeof(Message), 0);
    if (result != sizeof(Message)) {
        // if anything goes wrong we say bye bye to player , our game our rules
        player_removed(p);
    }
}

void send_to_other_players(Player* sender, Message message) {
    message.from_player = sender->player_number;
    for (int i = 0; i < gd->number_of_active_players; ++i) {
        Player* objective = &sv->players[i];
        if (objective->player_number != -1 && // not inactive
            sender->player_number != objective->player_number) // not self
            send_message(objective, message);
    }
}


void player_moved(Player *p, Message m){
    // todo change map
    send_to_other_players(p,m);
}

void player_removed(Player *p){
    pthread_mutex_lock(&locks->game_data);
    pthread_mutex_lock(&locks->server_internals);

    send_to_other_players(p,(Message){FROM_SERVER,PLAYER_DISCONNECTED,p->player_number});
    remove_player(p);

    pthread_mutex_unlock(&locks->server_internals);
    pthread_mutex_unlock(&locks->game_data);

}

// get a random value in the range [0, 1]
double rand01()
{
    return (double) rand() / (double) RAND_MAX;
}

void init_grid()
{
    if(grid==NULL){
        grid = calloc(GRIDSIZE, sizeof(TILETYPE*));
        for (int i = 0; i < GRIDSIZE; ++i) {
            grid[i] = calloc(GRIDSIZE, sizeof(TILETYPE));
        }
    }

    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            double r = rand01();
            if (r < 0.1) {
                grid[i][j] = TILE_TOMATO;
                gd->number_of_tomatoes++;
            }
            else
                grid[i][j] = TILE_GRASS;
        }
    }

    // force player's position to be grass
//    if (gd->grid[playerPosition.x][playerPosition.y] == TILE_TOMATO) {
//        grid[playerPosition.x][playerPosition.y] = TILE_GRASS;
//        numTomatoes--;
//    }

    // ensure grid isn't empty
    while (gd->number_of_tomatoes == 0)
        init_grid();
}

