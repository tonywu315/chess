#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

int start_search(Board *board, Parameter parameters);
int search_position(Board *board, Move *move, int time);

#endif
