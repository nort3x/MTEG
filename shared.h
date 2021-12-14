//
// Created by human on 12/7/21.
//

#ifndef CLIENT_SHARED_H
#define CLIENT_SHARED_H

#include "types.h"
#include "render.h"

Player* add_player_to_list(Player to_be_add, Player *ptr, int size_of_list);
PlayerStat * add_player_stat_to_list(PlayerStat to_be_add, PlayerStat *ptr, int size_of_list);

Player* remove_player_from_list(int index, Player *ptr, int size_of_list);
PlayerStat* remove_player_stat_from_list(int index, PlayerStat *ptr, int size_of_list);

void print_grid(TILETYPE(*)[GRIDSIZE]);
#endif //CLIENT_SHARED_H
