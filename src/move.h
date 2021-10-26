#ifndef MOVE_H
#define MOVE_H

#include "constants.h"

void move_piece(Move move);
void unmove_piece(Move move);
int move_legal(Move *move, Fast start, Fast end);

#endif