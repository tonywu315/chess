#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "constants.h"

Move create_move(Fast start, Fast end, Fast flag);
int is_attacking(Fast square, Fast player);
int in_check();
int generate_moves(Move *moves);

#endif