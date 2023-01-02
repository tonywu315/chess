#ifndef MOVE_ORDER_H
#define MOVE_ORDER_H

#include "types.h"

typedef struct moveList {
    Move move;
    int score;
} MoveList;

/*
    Move Ordering

    1. Transposition table move
    2. Winning captures with MVV/LVA
    3. Equal captures
    4. Killer moves
    5. Losing captures
    6. Quiet moves
*/
enum MoveValue {
    TT_MOVE = 100,
    WINNING_CAPTURE = 80,
    EQUAL_CAPTURE = 60,
    KILLER_MOVE = 40,
    LOSING_CAPTURE = 20,
    QUIET_MOVE = 0,
};

enum MVV_LVA {
    QxP = LOSING_CAPTURE,
    RxP,
    BxP,
    NxP,
    QxN,
    QxB,
    RxN,
    RxB,
    QxR,
    PxP = EQUAL_CAPTURE,
    BxN,
    NxN,
    BxB,
    NxB,
    RxR,
    QxQ,
    KxP = WINNING_CAPTURE,
    KxN,
    PxN,
    KxB,
    PxB,
    KxR,
    BxR,
    NxR,
    PxR,
    KxQ,
    RxQ,
    BxQ,
    NxQ,
    PxQ,
};

void score_moves(Board *board, Stack *stack, Move *moves, MoveList *move_list,
                 Move tt_move, int length);
void score_quiescence_moves(Board *board, Move *moves, MoveList *move_list,
                            int length);
Move sort_moves(MoveList *move_list, int length, int index);

#endif
