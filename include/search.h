#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

// Search parameters from the UCI protocol
typedef struct parameter {
    Move search_moves[MAX_MOVES];
    int move_count;
    U64 start_time;
    U64 move_time;
    U64 white_time;
    U64 black_time;
    U64 white_increment;
    U64 black_increment;
    int moves_to_go;
    int max_depth;
    int max_nodes;
    int mate;
    bool ponder;
    bool infinite;
} Parameter;

void start_search(Board *board, Parameter parameters);

#endif
