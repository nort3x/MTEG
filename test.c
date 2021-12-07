//
// Created by human on 12/7/21.
//

#include <malloc.h>
#include "package/shared.h"

int main(){


    Player * p = calloc(0, sizeof(Player));
    p = add_player_to_list((Player){0,2,3},p,0);
    p = add_player_to_list((Player){1,2,3},p,1);
    p = add_player_to_list((Player){2,2,3},p,2);

    p = remove_player_from_list(1,p,3);
    if(p[1].player_number == 2){
        printf("yes");
    }

    free(p);
    return 0;
}
