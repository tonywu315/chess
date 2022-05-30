#ifndef MOVE_H
#define MOVE_H

#include "types.h"

void print_move(Move move);
void make_move(Board *board, Move move);
void unmake_move(Board *board, Move move);

#endif
