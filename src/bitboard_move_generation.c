#include "bitboard_move_generation.h"
#include "bitboard.h"

const Bitboard MASK_FILE_A = UINT64_C(0xFEFEFEFEFEFEFEFE);
const Bitboard MASK_FILE_H = UINT64_C(0x7F7F7F7F7F7F7F7F);
const Bitboard MASK_FILE_AB = UINT64_C(0xFCFCFCFCFCFCFCFC);
const Bitboard MASK_FILE_HG = UINT64_C(0x3F3F3F3F3F3F3F3F);

void init_attacks() {
    for (int square = 0; square < 64; square++) {
        Bitboard piece = UINT64_C(0);

        set_bit(&piece, square);
        pawn_attacks[WHITE - 1][square] = get_pawn_attacks(piece, WHITE);
        pawn_attacks[BLACK - 1][square] = get_pawn_attacks(piece, BLACK);
        knight_attacks[square] = get_knight_attacks(piece);
        king_attacks[square] = get_king_attacks(piece);
    }
}

Bitboard get_pawn_attacks(Bitboard pawns, U8 player) {
    if (player == WHITE) {
        return ((pawns << 9) & MASK_FILE_A) | ((pawns << 7) & MASK_FILE_H);
    } else {
        return ((pawns >> 7) & MASK_FILE_A) | ((pawns >> 9) & MASK_FILE_H);
    }
}

Bitboard get_knight_attacks(Bitboard knights) {
    Bitboard left1 = (knights >> 1) & MASK_FILE_H;
    Bitboard left2 = (knights >> 2) & MASK_FILE_HG;
    Bitboard right1 = (knights << 1) & MASK_FILE_A;
    Bitboard right2 = (knights << 2) & MASK_FILE_AB;
    Bitboard height1 = left1 | right1;
    Bitboard height2 = left2 | right2;

    return (height1 << 16) | (height1 >> 16) | (height2 << 8) | (height2 >> 8);
}

Bitboard get_king_attacks(Bitboard king) {
    Bitboard attacks =
        ((king << 1) & MASK_FILE_A) | ((king >> 1) & MASK_FILE_H);

    king |= attacks;
    attacks |= (king << 8) | (king >> 8);

    return attacks;
}
