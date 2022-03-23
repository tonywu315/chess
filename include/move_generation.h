#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "types.h"

bool is_legal(Board *board, Move move);
int generate_moves(const Board *board, Move *moves);
int generate_quiescence_moves(const Board *board, Move *moves);
int generate_legal_moves(Board *board, Move *moves);

#endif
