#ifndef MOVE_H
#define MOVE_H

#include "constants.h"

void make_move(Board *board, Move move);
void unmake_move(Board *board, Move move);

// int move_legal(Board *board, Move move);
// int move_computer(int depth);

#endif
