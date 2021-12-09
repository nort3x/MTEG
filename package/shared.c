//
// Created by human on 12/7/21.
//

#include "shared.h"
#include <stdlib.h>

Player * remove_player_from_list(int index, Player *ptr, int size_of_list) {
    for (int i = index; i < size_of_list - 1; ++i) {
        ptr[i] = ptr[i + 1];
    }
    return realloc((void *) ptr, sizeof(Player) * (size_of_list - 1));
}

PlayerStat * remove_player_stat_from_list(int index, PlayerStat *ptr, int size_of_list) {
    for (int i = index; i < size_of_list - 1; ++i) {
        ptr[i] = ptr[i + 1];
    }
    return realloc((void *) ptr, sizeof(PlayerStat) * (size_of_list - 1));
}

Player* add_player_to_list(Player to_be_add, Player *ptr, int size_of_list){
    Player* new_list =  realloc((void *) ptr, sizeof(Player) * (size_of_list + 1));
    new_list[size_of_list] = to_be_add;
    return new_list;
}

PlayerStat * add_player_stat_to_list(PlayerStat to_be_add, PlayerStat *ptr, int size_of_list){
    PlayerStat * new_list =  realloc((void *) ptr, sizeof(PlayerStat) * (size_of_list + 1));
    new_list[size_of_list] = to_be_add;
    return new_list;
}