#ifndef BITBOARD_MOVE_GENERATION_H
#define BITBOARD_MOVE_GENERATION_H

#include "constants.h"

void init_attacks();
int is_attacked(Board board, U8 square, U8 player);

#endif
