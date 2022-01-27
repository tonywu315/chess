#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "types.h"

#define TRANSPOSITION_EXACT 0
#define TRANSPOSITION_ALPHA 1
#define TRANSPOSITION_BETA 2

void init_transposition(int megabytes);
void clear_transposition();
int get_transposition(U64 hash, int depth, int alpha, int beta, Move *move);
void set_transposition(U64 hash, int depth, int score, int flag, Move move);
U64 get_hash(Board *board);

#endif
