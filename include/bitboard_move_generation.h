#ifndef BITBOARD_MOVE_GENERATION_H
#define BITBOARD_MOVE_GENERATION_H

#include "constants.h"

extern const Bitboard MASK_FILE_A, MASK_FILE_H, MASK_FILE_AB, MASK_FILE_HG;
extern Bitboard pawn_attacks[2][64], knight_attacks[64], king_attacks[64],
    rook_attacks[64], bishop_attacks[64];

void init_attacks();
Bitboard get_pawn_attacks(Bitboard pawns, U8 player);
Bitboard get_knight_attacks(Bitboard knights);
Bitboard get_king_attacks(Bitboard king);

#endif
