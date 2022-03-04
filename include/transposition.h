#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "types.h"

#define EXACT_BOUND 0
#define LOWER_BOUND 1
#define UPPER_BOUND 2

void init_transposition(int megabytes);
void clear_transposition();
int get_transposition(U64 hash, int alpha, int beta, int ply, int depth,
                      Move *move);
void set_transposition(U64 hash, int score, int flag, int ply, int depth,
                       Move move);
U64 get_hash(Board *board);

#endif
