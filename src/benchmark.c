#include "benchmark.h"
#include "board.h"
#include "move.h"
#include "move_generation.h"

typedef unsigned long long U64;

/*
CURRENT PERFORMANCE FOR DEPTH 6
perft:          2.555 MNPS
speedy_perft:   16.651 MNPS
pseudo_perft:   21.696 MNPS
*/

/* Performance test for enumerating all moves to a certain depth */
U64 perft(int depth) {
    Move move_list[MAX_MOVES];
    U64 nodes = 0;

    if (depth == 0) {
        return 1ULL;
    }

    int count = generate_moves(move_list);

    for (int i = 0; i < count; i++) {
        move_piece(&move_list[i]);
        if (!in_check()) {
            nodes += perft(depth - 1);
        }
        unmove_piece();
    }
    return nodes;
}

/* Performance test for enumerating mostly legal moves to a certain depth */
U64 speedy_perft(int depth) {
    Move move_list[MAX_MOVES];
    U64 nodes = 0;
    int count = generate_moves(move_list);

    if (depth == 1) {
        return (U64)count;
    }

    for (int i = 0; i < count; i++) {
        move_piece(&move_list[i]);
        if (!in_check()) {
            nodes += speedy_perft(depth - 1);
        }
        unmove_piece();
    }
    return nodes;
}

/* Performance test for enumerating all pseudo legal moves to a certain depth */
U64 pseudo_perft(int depth) {
    Move move_list[MAX_MOVES];
    U64 nodes = 0;
    int count = generate_moves(move_list);

    if (depth == 1) {
        return (U64)count;
    }

    for (int i = 0; i < count; i++) {
        move_piece(&move_list[i]);
        nodes += pseudo_perft(depth - 1);
        unmove_piece();
    }
    return nodes;
}

/* Computes time to complete task */
void benchmark(int depth) {
    U64 nodes;
    double time;
    clock_t begin_time, end_time;

    start_board();

    begin_time = clock();
    nodes = pseudo_perft(depth);
    end_time = clock();
    time = (double)(end_time - begin_time) / CLOCKS_PER_SEC;

    printf("Depth %d, Nodes: %lld\n", depth, nodes);
    printf("Time: %lf seconds, MNPS: %.3f\n\n", time, nodes / (time * 1000000));
}