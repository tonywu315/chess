#ifndef MOVE_H
#define MOVE_H

#include "constants.h"

#define PROMOTION_FLAG 1
#define ENPASSANT_FLAG 2
#define CASTLING_FLAG 3

/*
    Information about a move is encoded in 16 bits

    Bits                   Information

    00 00 000000 111111    start square
    00 00 111111 000000    end square
    00 11 000000 000000    promotion piece
    11 00 000000 000000    special flags

    Special flags: promotion = 1, enpassant = 2, castling = 3
*/
typedef uint16_t Move;

void make_move(Board *board, Move move);
void unmake_move(Board *board, Move move);

// int move_legal(Board *board, Move move);
// int move_computer(int depth);

static inline Move encode_move(int start, int end, int flag, int promotion) {
    return start | (end << 6) | (flag << 12) | ((promotion - KNIGHT) << 14);
}

static inline int get_move_start(Move move) { return move & 0x3F; }
static inline int get_move_end(Move move) { return (move >> 6) & 0x3F; }
static inline int get_move_flag(Move move) { return (move >> 12) & 3; }
static inline int get_move_promotion(Move move) { return (move >> 14) & 3; }

#endif
