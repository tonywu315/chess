#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "constants.h"

void init_attacks();
int is_attacked(Board board, int square, int player);

#endif
