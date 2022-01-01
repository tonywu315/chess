#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "constants.h"

void print_move(Move move);
bool is_legal(Board *board, Move move);
int generate_moves(const Board *board, Move *moves);

#endif
