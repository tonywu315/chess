#ifndef MOVE_H
#define MOVE_H

#include "types.h"

enum Move_Type {
    NULLMOVE,
    PROMOTION,
    ENPASSANT,
    CASTLING,
};

void print_move(Move move);
void make_move(Board *board, Move move);
void unmake_move(Board *board, Move move);

#endif
