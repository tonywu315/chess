#include "benchmark.h"
#include "board.h"
#include "move.h"
#include "move_generation.h"

/*
CURRENT PERFORMANCE FOR DEPTH 6
perft:          4.882 MNPS
speedy_perft:   29.444 MNPS
pseudo_perft:   35.542 MNPS
loop_speed:     376.165 MNPS
*/

/* Performance test for enumerating all moves to a certain depth */
void perft(int depth, U64 *nodes) {
    Move move_list[MAX_MOVES];

    if (depth == 0) {
        *nodes += 1ULL;
    } else {
        int count = generate_moves(move_list);

        for (int i = 0; i < count; i++) {
            move_piece(&move_list[i]);
            if (!in_check(3 - board.player)) {
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
            if (!in_check(3 - board.player)) {
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

/* Tests speed to loop through depth * 1 billion times */
static inline U64 loop_speed(int depth) {
    U64 billion = 1000000000;
    for (U64 i = 0; i < depth * billion; i++) {
        ;
    }

    return depth * billion;
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
