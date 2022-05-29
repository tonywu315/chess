#ifndef GAME_H
#define GAME_H

#include "types.h"

enum Status {
    NO_STATUS,
    WHITE_WIN,
    BLACK_WIN,
    DRAW,
};

void start_singleplayer(Board *board, bool player_first, int time);
void start_multiplayer(Board *board);

#endif
