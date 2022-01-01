#ifndef ATTACKS_H
#define ATTACKS_H

#include "types.h"

void init_attacks();
Bitboard get_attacks(const Board *board, int square, int piece);
bool is_attacked(const Board *board, int square, int player);
bool in_check(const Board *board, int player);

#endif
