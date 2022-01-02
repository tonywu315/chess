
#include "benchmark.h"
#include "attacks.h"
#include "bitboard.h"
#include "move.h"
#include "move_generation.h"

/*
    CURRENT PERFORMANCE FOR DEPTH 6
    perft:           61.145 MNPS
    make_unmake:     115.302 MNPS

    Bulk counting:
    speedy_perft:    354.398 MNPS
    pseudo_perft:    370.105 MNPS
*/

static inline void speedy_perft(Board *board, int depth, U64 *nodes);
static inline void pseudo_perft(Board *board, int depth, U64 *nodes);
static inline void make_unmake(Board *board, int depth, U64 *nodes);

// Compute time to complete task
void benchmark(Board *board, int depth) {
    U64 nodes = 0;
    double time;
    clock_t begin_time, end_time;

    begin_time = clock();
    pseudo_perft(board, depth, &nodes);
    end_time = clock();
    time = (double)(end_time - begin_time) / CLOCKS_PER_SEC;

    printf("Depth %d, Nodes: %lld\n", depth, nodes);
    printf("Time: %lf seconds, MNPS: %.3f\n", time, nodes / (time * 1000000));
}

// Performance test for enumerating all moves to a certain depth
void perft(Board *board, int depth, U64 *nodes) {
    Move moves[MAX_MOVES];

    if (depth == 0) {
        *nodes += 1ULL;
    } else {
        int count = generate_moves(board, moves);

        for (int i = 0; i < count; i++) {
            make_move(board, moves[i]);
            if (!in_check(board, !board->player)) {
                perft(board, depth - 1, nodes);
            }
            unmake_move(board, moves[i]);
        }
    }
}

// Performance test for enumerating mostly legal moves to a certain depth
static inline void speedy_perft(Board *board, int depth, U64 *nodes) {
    Move moves[MAX_MOVES];
    int count = generate_moves(board, moves);

    if (depth == 1) {
        *nodes += (U64)count;
    } else {
        for (int i = 0; i < count; i++) {
            make_move(board, moves[i]);
            if (!in_check(board, !board->player)) {
                speedy_perft(board, depth - 1, nodes);
            }
            unmake_move(board, moves[i]);
        }
    }
}

// Performance test for enumerating all pseudo legal moves to a certain depth
static inline void pseudo_perft(Board *board, int depth, U64 *nodes) {
    Move moves[MAX_MOVES];
    int count = generate_moves(board, moves);

    if (depth == 1) {
        *nodes += (U64)count;
    } else {
        for (int i = 0; i < count; i++) {
            make_move(board, moves[i]);
            pseudo_perft(board, depth - 1, nodes);
            unmake_move(board, moves[i]);
        }
    }
}

// Tests speed to make and unmake depth * 100 million times
static inline void make_unmake(Board *board, int depth, U64 *nodes) {
    U64 hundred_million = 100000000;
    Move move = encode_move(E2, E4, 0, 0);
    for (U64 i = 0; i < depth * hundred_million; i++) {
        make_move(board, move);
        *nodes += 1;
        unmake_move(board, move);
    }
}
