#ifndef MOVE_H
#define MOVE_H

#include "types.h"

void make_move(Board *board, Move move);
void unmake_move(Board *board, Move move);
int move_legal(Board *board, Move move);
int move_computer(Board *board, int depth);

#endif
