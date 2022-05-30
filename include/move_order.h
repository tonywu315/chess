#ifndef MOVE_ORDER_H
#define MOVE_ORDER_H

#include "types.h"

typedef struct moveList {
    Move move;
    int score;
} MoveList;

/*
    Move Ordering

    1. Hash Move
    2. Winning captures
    3. Equal captures
    4. Killer moves
    5. Losing captures
    6. Quiet moves
*/
enum MoveValue {
    TT_MOVE = 100,
    KILLER_MOVE = -1,
    QUIET_MOVE = -100,
};

void score_moves(Board *board, Move *moves, MoveList *move_list, int length,
                 Move tt_move);
Move sort_moves(MoveList *move_list, int length, int index);

#endif
