#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VERSION 1.0

#define MAX_PLY 64
#define MAX_MOVES 1024
#define ARRAY_SIZE 128
#define SUCCESS 0
#define FAILURE 1

#define CASTLE_WK 1
#define CASTLE_WQ 2
#define CASTLE_BK 4
#define CASTLE_BQ 8

/* Debug macro only appears if DEBUG is passed in */
/* Compiler optimizes out functions if DEBUG_VALUE is not set */
#ifdef DEBUG
#define DEBUG_VALUE true
#else
#define DEBUG_VALUE false
#endif

/* Prints debug information */
#define debug_printf(fmt, ...)                                                 \
    do {                                                                       \
        if (DEBUG_VALUE)                                                       \
            fprintf(stderr, "[%s] %s:%d in %s(): " fmt, __TIME__, __FILE__,    \
                    __LINE__, __func__, __VA_ARGS__);                          \
    } while (0)

/* Asserts that expression is true and exits if it is not */
#define debug_assert(expression)                                               \
    do {                                                                       \
        if (DEBUG_VALUE && !(expression)) {                                    \
            fprintf(stderr, "[%s] %s:%d in %s(): %s FAILED\n", __TIME__,       \
                    __FILE__, __LINE__, __func__, #expression);                \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

typedef uint64_t Bitboard;
typedef unsigned long long U64;

typedef struct state {
    int capture;
    int castling;
    int enpassant;
    int draw_ply;
} State;

typedef struct board {
    State state[MAX_MOVES];
    Bitboard pieces[16];
    Bitboard occupancies[3];
    int board[64];
    int player;
    int ply;
} Board;

// typedef struct oldboard {
//     int colors[ARRAY_SIZE];
//     int pieces[ARRAY_SIZE];
//     int player;    /* Player to move */
//     int castle;    /* 0-15 number that represents castling availability */
//     int enpassant; /* En passant square */
//     int ply;       /* Keeps track for 50 move rule */
//     int king[2];   /* Location of kings */
// } OldBoard;

// extern OldBoard oldboard;
// typedef struct move {
//     int start;
//     int end;
//     int captured;
//     int flag;
//     int castle;
//     int enpassant;
//     int ply;
// } OldMove;

// extern OldMove game_moves[MAX_MOVES]; /* May need to reallocate rarely */
// extern int game_position;

// typedef struct line {
//     int length;
//     OldMove moves[MAX_PLY];
// } Line;

// clang-format off
enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE
};

enum Color {
    WHITE,
    BLACK,
    NO_COLOR
};

enum Piece_type {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NO_PIECE_TYPE
};

enum Piece {
    W_PAWN = PAWN,     W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    NO_PIECE = 15,
};

enum Direction {
    UPRIGHT = 9,
    UP = 8,
    UPLEFT = 7,
    RIGHT = 1,
    LEFT = -1,
    DOWNRIGHT = -7,
    DOWN = -8,
    DOWNLEFT = -9
};

// enum Flag {
//     NORMAL = 0,
//     CAPTURE = 3,
//     DOUBLE = 5,
//     ENPASSANT = 6,
//     NULLMOVE = 7,
//     CASTLE_WK = 1,
//     CASTLE_WQ = 2,
//     CASTLE_BK = 4,
//     CASTLE_BQ = 8,
//     PROMOTION_N = 9,
//     PROMOTION_B = 10,
//     PROMOTION_R = 11,
//     PROMOTION_Q = 12
// };

enum Result {
    NONE,
    WHITE_WIN,
    BLACK_WIN,
    DRAW
};

static inline int get_piece(int piece) { return piece & 7; }
static inline int get_color(int piece) { return piece >> 3; }

/* Bitboard Functions */

static inline int get_bit(Bitboard bitboard, int square) {
    return bitboard & (UINT64_C(1) << square) ? 1 : 0;
}

static inline void set_bit(Bitboard *bitboard, int square) {
    *bitboard |= UINT64_C(1) << square;
}

static inline void flip_bit(Bitboard *bitboard, int square) {
    *bitboard ^= UINT64_C(1) << square;
}

static inline void clear_bit(Bitboard *bitboard, int square) {
    *bitboard &= ~(UINT64_C(1) << square);
}

static inline Bitboard create_bit(int square) { return UINT64_C(1) << square; }

static inline int in_bounds(int start, int direction) {
    int end = start + direction;
    int distance = abs((start / 8 - end / 8) - (start % 8 - end % 8));
    return end >= A1 && end <= H8 && distance <= 2;
}

/* Hamming Weight Algorithm, 12 Arithmetic Operations */
static inline int get_population(Bitboard bitboard) {
    /* 2-adic fractions: -1/3, -1/5, -1/17, -1/255 */
    const Bitboard k1 = UINT64_C(0x5555555555555555);
    const Bitboard k2 = UINT64_C(0x3333333333333333);
    const Bitboard k3 = UINT64_C(0x0F0F0F0F0F0F0F0F);
    const Bitboard kf = UINT64_C(0x0101010101010101);

    bitboard -= ((bitboard >> 1) & k1);
    bitboard = (bitboard & k2) + ((bitboard >> 2) & k2);
    bitboard = (bitboard + (bitboard >> 4)) & k3;
    return (int)((bitboard * kf) >> 56);
}

/* ~35% faster for 1 bit, similar for 2 bits */
static inline int get_sparse_population(Bitboard bitboard) {
    int count = 0;
    while (bitboard) {
        bitboard &= bitboard - 1;
        count++;
    }
    return count;
}

// int pop_lsb(Bitboard *bitboard) {
//     int lsb = get_lsb(*bitboard);
//     *bitboard &= *bitboard - 1;
//     return lsb;
// }

// int get_lsb(Bitboard bitboard) {
//     static const int lsb_table[64] = {
//         63, 30, 3,  32, 59, 14, 11, 33, 60, 24, 50, 9,  55, 19, 21, 34,
//         61, 29, 2,  53, 51, 23, 41, 18, 56, 28, 1,  43, 46, 27, 0,  35,
//         62, 31, 58, 4,  5,  49, 54, 6,  15, 52, 12, 40, 7,  42, 45, 16,
//         25, 57, 48, 13, 10, 39, 8,  44, 20, 47, 38, 22, 17, 37, 36, 26};

//     bitboard ^= bitboard - 1;
//     return lsb_table[((int)bitboard ^ (bitboard >> 32)) * 0x78291ACF >> 26];
// }

/* 0x88 Board Representation (16x8 array) */

// /* Returns file and rank of square (number from 0 to 7) */
// static inline int get_file(int square) { return square & 7; }
// static inline int get_rank(int square) { return square >> 4; }

// /* Returns square given file and rank */
// static inline int get_square(int file, int rank) { return file + rank * 16; }

// /* Returns true if square is outside the board */
// static inline bool invalid_square(int square) { return square & 0x88; }

// /* Returns true if file or rank is valid */
// static inline bool valid_row(int row) { return row <= 7; }

// /* Checks if a certain player has a certain piece at square */
// static inline bool exists(int square, int player, int piece) {
//     return oldboard.pieces[square] == piece &&
//            oldboard.colors[square] == player;
// }

#endif
