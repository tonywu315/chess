#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

int search(Board *board, int alpha, int beta, int ply, int depth,
           Line *mainline);

#endif
