#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define VERSION "2.2"

#define MAX_DEPTH 64
#define MAX_MOVES 256
#define MAX_GAME_LENGTH 1024
#define SUCCESS 0
#define FAILURE 1

#define INFINITY 30000
#define INVALID_SCORE 32767

#define CASTLE_WK 1
#define CASTLE_WQ 2
#define CASTLE_BK 4
#define CASTLE_BQ 8

#define NULLMOVE 0
#define PROMOTION 1
#define ENPASSANT 2
#define CASTLING 3

// Debug flag
#ifdef DEBUG
#define DEBUG_FLAG true
#else
#define DEBUG_FLAG false
#endif

// Log flag
#ifdef LOG
#define LOG_FLAG true
#else
#define LOG_FLAG false
#endif

// Prints debug information
#define debug_printf(fmt, ...)                                                 \
    do {                                                                       \
        if (DEBUG_FLAG)                                                        \
            fprintf(stderr, "[%s] %s:%d in %s(): " fmt, __TIME__, __FILE__,    \
                    __LINE__, __func__, __VA_ARGS__);                          \
    } while (0)

/*
    Information about a move is encoded in 16 bits

    Bits                   Information

    00 00 000000 111111    start square
    00 00 111111 000000    end square
    00 11 000000 000000    special flags
    11 00 000000 000000    promotion piece

    Special flags: promotion = 1, enpassant = 2, castling = 3

    For castling moves, start square is the king and end square is the rook
*/
typedef uint16_t Move;

// each of the 64 bits represents a square on the board
typedef uint64_t Bitboard;
typedef uint64_t U64;

typedef struct state {
    int capture;
    int castling;
    int enpassant;
    int draw_ply;
} State;

extern int collisions;

typedef struct board {
    State state[MAX_GAME_LENGTH];
    U64 hash;
    Bitboard pieces[16];
    Bitboard occupancies[3];
    int board[64];
    int player;
    int ply;
} Board;

typedef struct line {
    int length;
    Move moves[MAX_DEPTH];
} Line;

typedef struct transposition {
    Bitboard hash;
    Move move;
    int16_t score;
    uint8_t age;
    uint8_t depth;
    uint8_t flag;
    bool pv_node;
} Transposition;

// Global shared transposition table
extern Transposition *transposition;
extern U64 transposition_size;

extern int game_ply;
extern U64 qnodes;
extern bool time_over;

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

enum Result {
    NONE,
    WHITE_WIN,
    BLACK_WIN,
    DRAW
};
// clang-format on

// Get string coordinates from square
static inline char *get_coordinates(int square) {
    // clang-format off
    static char coordinates[64][3] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    };
    // clang-format on
    return coordinates[square];
}

// Mate functions

static inline bool is_mate_score(int score) {
    return abs(score) >= INFINITY - 100;
}
static inline int score_to_mate(int score) {
    return (INFINITY - abs(score)) / 2;
}

// Move and piece functions

static inline Move encode_move(int start, int end, int flag, int promotion) {
    return start | (end << 6) | (flag << 12) | ((promotion - KNIGHT) << 14);
}

static inline int get_move_start(Move move) { return move & 0x3F; }
static inline int get_move_end(Move move) { return (move >> 6) & 0x3F; }
static inline int get_move_flag(Move move) { return (move >> 12) & 3; }
static inline int get_move_promotion(Move move) { return (move >> 14) & 3; }

static inline int get_piece(int piece) { return piece & 7; }
static inline int get_color(int piece) { return piece >> 3; }
static inline int make_piece(int piece, int color) {
    return piece | (color << 3);
}

// Bitboard functions

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

static inline Bitboard shift_bit(Bitboard bitboard, int direction) {
    static const Bitboard MASK_FILE_A = UINT64_C(0xFEFEFEFEFEFEFEFE);
    static const Bitboard MASK_FILE_H = UINT64_C(0x7F7F7F7F7F7F7F7F);
    return direction == UP          ? bitboard << 8
           : direction == DOWN      ? bitboard >> 8
           : direction == LEFT      ? (bitboard >> 1) & MASK_FILE_H
           : direction == RIGHT     ? (bitboard << 1) & MASK_FILE_A
           : direction == UPLEFT    ? (bitboard << 7) & MASK_FILE_H
           : direction == UPRIGHT   ? (bitboard << 9) & MASK_FILE_A
           : direction == DOWNLEFT  ? (bitboard >> 9) & MASK_FILE_H
           : direction == DOWNRIGHT ? (bitboard >> 7) & MASK_FILE_A
                                    : 0;
}

static inline bool in_bounds(int start, int direction) {
    int end = start + direction;
    int distance = abs((start / 8 - end / 8) - (start % 8 - end % 8));
    return end >= A1 && end <= H8 && distance <= 2;
}

static inline bool valid_row(int row) { return row >= 0 && row <= 7; }
static inline int make_square(int file, int rank) { return file + (rank << 3); }

// Pseudo random number generator
static inline Bitboard rand64() {
    // Fastest seed out of 10 billion starting seeds for magic number generation
    static Bitboard seed = UINT64_C(0xAE793F42471A8799);

    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    return seed * UINT64_C(0x2545F4914F6CDD1D);
}

// GCC/Clang compatible compiler
#if defined(__GNUC__)

// Get number of bits set
static inline int get_population(Bitboard bitboard) {
    return __builtin_popcountll(bitboard);
}

// Get index of least significant bit
static inline int get_lsb(Bitboard bitboard) {
    return __builtin_ctzll(bitboard);
}

// Microsoft C/C++ compatible compiler
#elif defined(_MSC_VER) && defined(_WIN64)

#include <nmmintrin.h>
// Get number of bits set
static inline int get_population(Bitboard bitboard) {
    return (int)_mm_popcnt_u64(bitboard);
}

#include <intrin.h>
// Get index of least significant bit
static inline int get_lsb(Bitboard bitboard) {
    unsigned long index;
    return _BitScanForward64(&index, bitboard);
    return (int)index;
}

#else

// Hamming Weight Algorithm, 12 Arithmetic Operations
static inline int get_population(Bitboard bitboard) {
    // 2-adic fractions: -1/3, -1/5, -1/17, -1/255
    const Bitboard k1 = UINT64_C(0x5555555555555555);
    const Bitboard k2 = UINT64_C(0x3333333333333333);
    const Bitboard k3 = UINT64_C(0x0F0F0F0F0F0F0F0F);
    const Bitboard kf = UINT64_C(0x0101010101010101);

    bitboard -= ((bitboard >> 1) & k1);
    bitboard = (bitboard & k2) + ((bitboard >> 2) & k2);
    bitboard = (bitboard + (bitboard >> 4)) & k3;
    return (int)((bitboard * kf) >> 56);
}

// Bitscan using De Bruijn multiplication
static inline int get_lsb(Bitboard bitboard) {
    static const int index[64] = {
        0,  1,  48, 2,  57, 49, 28, 3,  61, 58, 50, 42, 38, 29, 17, 4,
        62, 55, 59, 36, 53, 51, 43, 22, 45, 39, 33, 30, 24, 18, 12, 5,
        63, 47, 56, 27, 60, 41, 37, 16, 54, 35, 52, 21, 44, 32, 23, 11,
        46, 26, 40, 15, 34, 20, 31, 10, 25, 14, 19, 9,  13, 8,  7,  6};
    static const Bitboard debruijn64 = UINT64_C(0x03F79D71B4CB0A89);
    return index[((bitboard & -bitboard) * debruijn64) >> 58];
}

#endif

// Pop the least significant bit
static inline int pop_lsb(Bitboard *bitboard) {
    int lsb = get_lsb(*bitboard);
    *bitboard &= *bitboard - 1;
    return lsb;
}

#endif
