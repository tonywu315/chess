#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "constants.h"

Move create_move(U8 start, U8 end, U8 flag);
int is_attacking(U8 square, U8 player);
int in_check();
int generate_moves(Move *moves);

#endif