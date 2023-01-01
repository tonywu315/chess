#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "types.h"

#define TT_HIT 32000
#define NO_TT_HIT 32001

// Transposition table entry
typedef struct transposition {
    U64 hash;
    Move move;
    int16_t score;
    uint8_t age;
    uint8_t depth;
    uint8_t flag;
    bool pv_node;
} Transposition;

enum Bound {
    NULL_BOUND,
    EXACT_BOUND,
    LOWER_BOUND,
    UPPER_BOUND,
};

void init_transposition(int megabytes);
void clear_transposition();
void free_transposition();
int get_transposition(U64 hash, int alpha, int beta, int ply, int depth,
                      Move *move);
void set_transposition(U64 hash, int score, int flag, int ply, int depth,
                       Move move);
void set_pv_moves(Board *board, Stack *stack, int score);
void get_pv_moves(Board *board);
U64 get_hash(Board *board);

#endif
