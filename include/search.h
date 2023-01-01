#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

// Search parameters from the UCI protocol
typedef struct parameter {
    Move search_moves[MAX_MOVES];
    U64 start_time;
    int move_count;
    int white_time;
    int black_time;
    int white_increment;
    int black_increment;
    int moves_to_go;
    int max_depth;
    int max_nodes;
    int mate;
    int move_time;
    bool ponder;
    bool infinite;
} Parameter;

void start_search(Board *board, Parameter parameters);

#endif
