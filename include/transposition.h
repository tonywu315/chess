#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "types.h"

#define TT_HIT 32000
#define NO_TT_HIT 32001

enum Bound {
    NULL_BOUND,
    EXACT_BOUND,
    LOWER_BOUND,
    UPPER_BOUND,
};

void init_transposition(int megabytes);
void clear_transposition();
int get_transposition(U64 hash, int alpha, int beta, int ply, int depth,
                      Move *move);
void set_transposition(U64 hash, int score, int flag, int ply, int depth,
                       Move move);
void set_pv_moves(Board *board, Line *mainline, int score);
void get_pv_moves(Board *board);
U64 get_hash(Board *board);

#endif
