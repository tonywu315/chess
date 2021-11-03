#ifndef MOVE_H
#define MOVE_H

#include "constants.h"

void move_piece(const Move *move);
void unmove_piece();
int move_legal(Fast start, Fast end, Fast promotion);

#endif