#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "types.h"

void init_transposition(int megabytes);
void clear_transposition();
Bitboard get_hash(Board *board);

#endif
