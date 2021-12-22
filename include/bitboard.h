#ifndef BITBOARD_H
#define BITBOARD_H

#include "constants.h"

extern const Bitboard FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G,
    FILE_H, RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8;

void print_bitboard(Bitboard bitboard);

#endif
