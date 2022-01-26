#include "transposition.h"

static Bitboard piece_square_key[12][64];
static Bitboard castling_key[16];
static Bitboard enpassant_key[64 + 1];
static Bitboard side_key[2];

void init_hash_keys() {
    for (int square = A1; square <= H8; square++) {
        for (int piece = PAWN; piece <= KING + 6; piece++) {
            piece_square_key[piece][square] = rand64();
        }
        enpassant_key[square] = rand64();
    }
    enpassant_key[NO_SQUARE] = 0;

    for (int castling = 0; castling < 16; castling++) {
        castling_key[castling] = rand64();
    }

    side_key[0] = 0;
    side_key[1] = rand64();
}

Bitboard get_hash(Board *board) {
    Bitboard hash_key = UINT64_C(0), pieces;
    
    for (int piece = PAWN; piece <= KING + 6; piece++) {
        pieces = board->pieces[piece];
        while (pieces) {
            hash_key ^= piece_square_key[piece][pop_lsb(&pieces)];
        }
    }

    hash_key ^= castling_key[board->state[0].castling];
    hash_key ^= enpassant_key[board->state[0].enpassant];
    hash_key ^= side_key[board->player];

    return hash_key;
}
