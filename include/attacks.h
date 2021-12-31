#ifndef ATTACKS_H
#define ATTACKS_H

#include "constants.h"

void init_attacks();
Bitboard get_attacks(Board board, int square);
bool is_attacked(Board board, int square, int player);

#endif
