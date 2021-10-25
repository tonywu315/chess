#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_MOVES 256
#define ARRAY_SIZE 128
#define ALL_CASTLE 15
#define SUCCESS 0
#define FAILURE 1
#define TRUE 1
#define FALSE 0

/* 0x88 Board Representation (16x8 array) */
typedef struct board {
    char colors[ARRAY_SIZE];
    char pieces[ARRAY_SIZE];
    char player;        /* Player to move */
    char enpassant;     /* En passant file */
    char castle;        /* 0-15 number that represents castling availability */
    char ply;           /* Keeps track for 50 move rule */
} Board;

extern Board board;

typedef struct move {
    char start;
    char end;
    char flag;
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

enum directions {
    UPRIGHT = 17,
    UP = 16,
    UPLEFT = 15,
    RIGHT = 1,
    LEFT = -1,
    DOWNRIGHT = -15,
    DOWN = -16,
    DOWNLEFT = -17,
};

enum flags {
    NORMAL,
    ENPASSANT,
    CASTLE_WK,
    CASTLE_WQ,
    CASTLE_BK,
    CASTLE_BQ,
    PROMOTION_N,
    PROMOTION_B,
    PROMOTION_R,
    PROMOTION_Q,
};

#endif