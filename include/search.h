#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

void start_search(Board *board, Parameter parameters);
int search_position(Board *board, Move *move, int time);

#endif
