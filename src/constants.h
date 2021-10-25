#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdio.h>

#define MAX_MOVES 256
#define ARRAY_SIZE 128
#define ALL_CASTLE 15
#define SUCCESS 0
#define FAILURE 1
#define TRUE 1
#define FALSE 0

/* 0x88 Board Representation (16x8 array) */
typedef struct board {
    unsigned char colors[ARRAY_SIZE];
    unsigned char pieces[ARRAY_SIZE];
    unsigned char player;        /* Player to move */
    unsigned char castle;        /* 0-15 number that represents castling availability */
    unsigned char enpassant;     /* En passant square */
    unsigned char ply;           /* Keeps track for 50 move rule */
    unsigned char king[2];       /* Location of kings */
} Board;

extern Board board;

typedef struct move {
    unsigned char start;
    unsigned char end;
    unsigned char captured;
    unsigned char flag;
    unsigned char enpassant;
    unsigned char ply;
} Move;

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
    CASTLE_WK = 1,
    CASTLE_WQ = 2,
    CASTLE_BK = 4,
    CASTLE_BQ = 8,
    PROMOTION_N = 9,
    PROMOTION_B = 10,
    PROMOTION_R = 11,
    PROMOTION_Q = 12
};

#endif