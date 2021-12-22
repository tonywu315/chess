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
#define MAX_MOVES 256
#define MAX_GAME_LENTH 1024
#define ARRAY_SIZE 128
#define ALL_CASTLE 15
#define SUCCESS 0
#define FAILURE 1

/* Debug macro only appears if DEBUG is passed in */
/* Compiler optimizes out functions if DEBUG_VALUE is not set */
#ifdef DEBUG
#define DEBUG_VALUE true
#else
#define DEBUG_VALUE false
#endif

/* Prints debug information */
#define debug_print(fmt, ...)                                                  \
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
typedef uint_fast8_t U8;
typedef unsigned long long U64;

typedef struct board {
    U8 colors[ARRAY_SIZE];
    U8 pieces[ARRAY_SIZE];
    U8 player;    /* Player to move */
    U8 castle;    /* 0-15 number that represents castling availability */
    U8 enpassant; /* En passant square */
    U8 ply;       /* Keeps track for 50 move rule */
    U8 king[2];   /* Location of kings */
} Board;

extern Board board;
typedef struct move {
    U8 start;
    U8 end;
    U8 captured;
    U8 flag;
    U8 castle;
    U8 enpassant;
    U8 ply;
} Move;

extern Move game_moves[MAX_GAME_LENTH]; /* May need to reallocate rarely */
extern int game_position;

typedef struct line {
    int length;
    Move moves[MAX_PLY];
} Line;

// clang-format off
enum square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
};

// enum square {
//     A1 = 0  , B1, C1, D1, E1, F1, G1, H1,
//     A2 = 16 , B2, C2, D2, E2, F2, G2, H2,
//     A3 = 32 , B3, C3, D3, E3, F3, G3, H3,
//     A4 = 48 , B4, C4, D4, E4, F4, G4, H4,
//     A5 = 64 , B5, C5, D5, E5, F5, G5, H5,
//     A6 = 80 , B6, C6, D6, E6, F6, G6, H6,
//     A7 = 96 , B7, C7, D7, E7, F7, G7, H7,
//     A8 = 112, B8, C8, D8, E8, F8, G8, H8
// };

enum color {
    EMPTY_COLOR,
    WHITE,
    BLACK
};

enum piece {
    EMPTY_PIECE,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum direction {
    UPRIGHT = 9,
    UP = 8,
    UPLEFT = 7,
    RIGHT = 1,
    LEFT = -1,
    DOWNRIGHT = -7,
    DOWN = -8,
    DOWNLEFT = -9
};

// enum direction {
//     UPRIGHT = 17,
//     UP = 16,
//     UPLEFT = 15,
//     RIGHT = 1,
//     LEFT = -1,
//     DOWNRIGHT = -15,
//     DOWN = -16,
//     DOWNLEFT = -17
// };

enum flag {
    NORMAL = 0,
    CAPTURE = 3,
    DOUBLE = 5,
    ENPASSANT = 6,
    NULLMOVE = 7,
    CASTLE_WK = 1,
    CASTLE_WQ = 2,
    CASTLE_BK = 4,
    CASTLE_BQ = 8,
    PROMOTION_N = 9,
    PROMOTION_B = 10,
    PROMOTION_R = 11,
    PROMOTION_Q = 12
};

enum result {
    NONE,
    WHITE_WIN,
    BLACK_WIN,
    DRAW
};
// clang-format on

/* Bitboard Functions */

static inline int get_bit(Bitboard bitboard, U8 square) {
    return bitboard & (UINT64_C(1) << square) ? 1 : 0;
}

static inline void set_bit(Bitboard *bitboard, U8 square) {
    *bitboard |= UINT64_C(1) << square;
}

static inline void flip_bit(Bitboard *bitboard, U8 square) {
    *bitboard ^= UINT64_C(1) << square;
}

static inline void clear_bit(Bitboard *bitboard, U8 square) {
    *bitboard &= ~(UINT64_C(1) << square);
}

/* Hamming Weight Algorithm, 12 Arithmetic Operations */
static inline int get_population(Bitboard bitboard) {
    /* 2-adic fractions: -1/3, -1/5, -1/17, -1/255 */
    const Bitboard K1 = UINT64_C(0x5555555555555555);
    const Bitboard K2 = UINT64_C(0x3333333333333333);
    const Bitboard K4 = UINT64_C(0x0F0F0F0F0F0F0F0F);
    const Bitboard KF = UINT64_C(0x0101010101010101);

    bitboard -= ((bitboard >> 1) & K1);
    bitboard = (bitboard & K2) + ((bitboard >> 2) & K2);
    bitboard = (bitboard + (bitboard >> 4)) & K4;
    return (int)((bitboard * KF) >> 56);
}

/* 0x88 Board Representation (16x8 array) */

/* Returns file and rank of square (number from 0 to 7) */
static inline int get_file(U8 square) { return square & 7; }
static inline int get_rank(U8 square) { return square >> 4; }

/* Returns square given file and rank */
static inline U8 get_square(U8 file, U8 rank) { return file + rank * 16; }

/* Returns true if square is outside the board */
static inline bool invalid_square(U8 square) { return square & 0x88; }

/* Returns true if file or rank is valid */
static inline bool valid_row(U8 row) { return row <= 7; }

/* Checks if a certain player has a certain piece at square */
static inline bool exists(U8 square, U8 player, U8 piece) {
    return board.pieces[square] == piece && board.colors[square] == player;
}

#endif
