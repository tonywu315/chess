#include "benchmark.h"
#include "board.h"
#include "move.h"
#include "move_generation.h"

typedef unsigned long long U64;

/*
CURRENT PERFORMANCE FOR DEPTH 6
perft:          2.658 MNPS
speedy_perft:   17.354 MNPS
pseudo_perft:   21.980 MNPS
loop_speed:     609.352 MNPS
*/

/* Performance test for enumerating all moves to a certain depth */
static inline void perft(int depth, U64 *nodes) {
    Move move_list[MAX_MOVES];

    if (depth == 0) {
        *nodes += 1ULL;
    } else {
        int count = generate_moves(move_list);

        for (int i = 0; i < count; i++) {
            move_piece(&move_list[i]);
            if (!in_check()) {
                perft(depth - 1, nodes);
            }
            unmove_piece();
        }
    }
}

/* Performance test for enumerating mostly legal moves to a certain depth */
static inline void speedy_perft(int depth, U64 *nodes) {
    Move move_list[MAX_MOVES];
    int count = generate_moves(move_list);

    if (depth == 1) {
        *nodes += (U64)count;
    } else {
        for (int i = 0; i < count; i++) {
            move_piece(&move_list[i]);
            if (!in_check()) {
                speedy_perft(depth - 1, nodes);
            }
            unmove_piece();
        }
    }
}

/* Performance test for enumerating all pseudo legal moves to a certain depth */
static inline void pseudo_perft(int depth, U64 *nodes) {
    Move move_list[MAX_MOVES];
    int count = generate_moves(move_list);

    if (depth == 1) {
        *nodes += (U64)count;
    } else {
        for (int i = 0; i < count; i++) {
            move_piece(&move_list[i]);
            pseudo_perft(depth - 1, nodes);
            unmove_piece();
        }
    }
}

/* Tests speed to loop through depth * 100 million times */
static inline U64 loop_speed(int depth) {
    for (U64 i = 0; i < (U64)(depth * 100000000); i++) {
        ;
    }

    return depth * 100000000;
}

/* Computes time to complete task */
void benchmark(int depth) {
    U64 nodes = 0;
    double time;
    clock_t begin_time, end_time;

    start_board();

    begin_time = clock();
    perft(depth, &nodes);
    end_time = clock();
    time = (double)(end_time - begin_time) / CLOCKS_PER_SEC;

    printf("Depth %d, Nodes: %lld\n", depth, nodes);
    printf("Time: %lf seconds, MNPS: %.3f\n\n", time, nodes / (time * 1000000));
}