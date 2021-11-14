#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PLY 64
#define MAX_MOVES 256
#define MAX_GAME_LENTH 1024
#define ARRAY_SIZE 128
#define ALL_CASTLE 15
#define SUCCESS 0
#define FAILURE 1
#define CHECKMATE_WHITE 2
#define CHECKMATE_BLACK 3
#define STALEMATE 1

/* Debug macro only appears if DEBUG is passed in */
#ifdef DEBUG
#define DEBUG_VALUE true
#else
#define DEBUG_VALUE false
#endif

#define debug_print(fmt, ...)                                                  \
    do {                                                                       \
        if (DEBUG_VALUE)                                                       \
            fprintf(stderr, "[%s] %s:%d in %s(): " fmt, __TIME__, __FILE__,    \
                    __LINE__, __func__, __VA_ARGS__);                          \
    } while (0)

typedef uint_fast8_t U8;
typedef unsigned long long U64;

/* 0x88 Board Representation (16x8 array) */
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
extern int root_pos;
extern int search_pos;

typedef struct line {
    int length;
    Move moves[MAX_PLY];
} Line;

enum square {
    A1 = 0  , B1, C1, D1, E1, F1, G1, H1,
    A2 = 16 , B2, C2, D2, E2, F2, G2, H2,
    A3 = 32 , B3, C3, D3, E3, F3, G3, H3,
    A4 = 48 , B4, C4, D4, E4, F4, G4, H4,
    A5 = 64 , B5, C5, D5, E5, F5, G5, H5,
    A6 = 80 , B6, C6, D6, E6, F6, G6, H6,
    A7 = 96 , B7, C7, D7, E7, F7, G7, H7,
    A8 = 112, B8, C8, D8, E8, F8, G8, H8
};

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
    UPRIGHT = 17,
    UP = 16,
    UPLEFT = 15,
    RIGHT = 1,
    LEFT = -1,
    DOWNRIGHT = -15,
    DOWN = -16,
    DOWNLEFT = -17
};

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

/* Returns true if square is outside the board */
static inline int invalid_square(U8 square) { return square & 0x88; }

/* Returns rank and file of square (number from 0 to 7) */
static inline int get_rank(U8 square) { return square >> 4; }
static inline int get_file(U8 square) { return square & 7; }

/* Checks if a certain player has a certain piece at square */
static inline int exists(U8 square, U8 player, U8 piece) {
    return board.pieces[square] == piece && board.colors[square] == player;
}

#endif
