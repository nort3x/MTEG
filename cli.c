// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include "package/types.h"
#include "package/render.h"
#include "package/shared.h"


typedef struct {
    int server_sock;
    int my_id;
    GameData gd;
    PlayerStat *stats;
    RenderData *rd;
} ClientData;

inline void connection_job(int, char const *[]);

inline void init();

inline void cleanup();

inline void do_initialize_client();

inline void register_listener_thread();


inline void render_game();

ClientData *cd;

int main(int argc, char const *argv[]) {
    init();
    connection_job(argc, argv);
    render_game();
    cleanup();
    return 0;
}

void init() {
    cd = malloc(sizeof(ClientData));
    cd->rd = malloc(sizeof(RenderData));
    cd->rd->grid = malloc(sizeof(TILETYPE) * GRIDSIZE * GRIDSIZE);
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
    register_listener_thread();
}

inline void recv_game_data();

inline void recv_self_id();

inline void recv_player_stats();

inline void post_init();

inline void recv_grid();

inline void make_render_data();

inline void process_message(Message m);

void do_initialize_client() {
    recv_game_data();
    recv_self_id();
    post_init();
    recv_player_stats();
    recv_grid();

    make_render_data();


    printf("my_id: %d \n", cd->my_id);


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
    } else {
        printf("game_data: { #ofPlayers:%d  current_score:%d current_level:%d } \n",
               cd->gd.number_of_active_players,
               cd->gd.current_score,
               cd->gd.current_level
        );
    }
}

void recv_self_id() {
    Message m;
    int i = recv(cd->server_sock, &m, sizeof(Message), 0);
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
    if (i != sizeof(PlayerStat) * cd->gd.number_of_active_players) {
        printf("could not receive player stats");
        exit(-1);
    }
}

void recv_grid() {
    long int i = recv(
            cd->server_sock,
            (cd->rd->grid),
            sizeof(TILETYPE) * GRIDSIZE * GRIDSIZE,
            0
    );
    if (i != sizeof(TILETYPE) * GRIDSIZE * GRIDSIZE) {
        printf("could not receive grid");
        exit(-1);
    } else {
        print_grid(cd->rd->grid);

    }
}

/*render and hooks*/

inline void move_left();

inline void move_right();

inline void move_top();

inline void move_bottom();


void render_game() {
    render(
            cd->rd,
            (MoveHooks) {move_left, move_right, move_top, move_bottom}
    );
}

void make_render_data() {
    cd->rd->gd = cd->gd;
    cd->rd->other_player = cd->stats;
    cd->rd->thisPlayer = cd->stats[cd->my_id];
}

void client_send_message(Code c,int data){
    Message m = {cd->my_id,c,data};
    if(send(cd->server_sock,&m, sizeof(Message),0) != sizeof(Message)){
        printf("sending message failed!\n");
    }else{
        printf("message sent: from:%d code:%d data:%d\n",m.from_player,m.code,m.data);
    }
}

void move_left(){
    client_send_message(MOVE_LEFT,0);
}
void move_right(){
    client_send_message(MOVE_RIGHT,0);
}
void move_top(){
    client_send_message(MOVE_FORWARD,0);
}
void move_bottom(){
    client_send_message(MOVE_BACKWARD,0);
}



void register_listener_thread(){

    Message  m;
    while (cd->my_id != -1){
        if(recv(cd->server_sock,&m, sizeof(Message),0) != sizeof(Message)){
            printf("Connection Lost\n");
            break;
        }
        process_message(m);
    }
    printf("trigger shouldExit for renderer\n");
    cd->rd->shouldExit = true;
}

inline void move_player(Message m);
inline void remove_player(Message m);
inline void update_game_data(Message m);


void process_message(Message m){
    switch (m.code) {
        case MOVE_BACKWARD:
        case MOVE_FORWARD:
        case MOVE_RIGHT:
        case MOVE_LEFT:
            move_player(m);
            break;
        case PLAYER_DISCONNECTED:
            remove_player(m);
            break;
        case LEVEL_UPDATE:
        case SCORE_UPDATE:
            update_game_data(m);
            break;
        case YOUR_ID:
            printf("should not receive this in this stage\n");
            break;
    }
}

int get_index_of_player_stat(int id) {
    int p_pos = -1;
    for (int i = 0; i < cd->gd.number_of_active_players; ++i) {
        if (cd->stats[i].id == id) {
            p_pos = i;
            break;
        }
    }
    return p_pos;
}

PlayerStat * get_objective(int id){
    int index = get_index_of_player_stat(id);
    if(index == -1){
        printf("player stat not found\n");
        return NULL;
    }
    return &cd->stats[index];
}

void move_player(Message m){
    PlayerStat * objective_player = get_objective(m.from_player);
    if(objective_player==NULL) return;

    switch (m.code) {
        case MOVE_LEFT:
            objective_player->p.x++;break;
        case MOVE_RIGHT:
            objective_player->p.x--;break;
        case MOVE_FORWARD:
            objective_player->p.y++;break;
        case MOVE_BACKWARD:
            objective_player->p.y--;break;
        default:
            printf("why i'm receiving this message?! it's not a movement message!");
    }


}
void remove_player(Message m){
    int id = get_index_of_player_stat(m.data);
    if(id==-1){
        printf("id not found\n");
        return;
    }
    cd->stats = remove_player_stat_from_list(id,cd->stats,cd->gd.number_of_active_players);
    cd->gd.number_of_active_players--;
    make_render_data();
}

void update_game_data(Message m){
    switch (m.code) {
        case SCORE_UPDATE:
            cd->gd.current_score = m.data;
            break;
        case LEVEL_UPDATE:
            cd->gd.current_level = m.data;
            break;

        default:
            printf("not an update message why i'm receiving it ?\n");
    }


}