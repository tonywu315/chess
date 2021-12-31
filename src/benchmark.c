#include "benchmark.h"
#include "attacks.h"
#include "bitboard.h"
#include "move.h"
#include "move_generation.h"

/*
CURRENT PERFORMANCE FOR DEPTH 6
perft:          55.419 MNPS
speedy_perft:   301.349 MNPS
pseudo_perft:   324.450 MNPS
loop_speed:     1126.715 MNPS
*/

static inline void perft(Board *board, int depth, U64 *nodes);
static inline void speedy_perft(Board *board, int depth, U64 *nodes);
static inline void pseudo_perft(Board *board, int depth, U64 *nodes);
static inline U64 loop_speed(int depth, U64 *nodes);


/* Computes time to complete task */
void benchmark(Board *board, int depth) {
    U64 nodes = 0;
    double time;
    clock_t begin_time, end_time;

    begin_time = clock();
    perft(board, depth, &nodes);
    end_time = clock();
    time = (double)(end_time - begin_time) / CLOCKS_PER_SEC;

    printf("Depth %d, Nodes: %lld\n", depth, nodes);
    printf("Time: %lf seconds, MNPS: %.3f\n", time, nodes / (time * 1000000));
}

/* Performance test for enumerating all moves to a certain depth */
static inline void perft(Board *board, int depth, U64 *nodes) {
    Move moves[MAX_MOVES];

    if (depth == 0) {
        *nodes += 1ULL;
    } else {
        int count = generate_moves(board, moves);

        for (int i = 0; i < count; i++) {
            make_move(board, moves[i]);
            if (!is_attacked(
                    board,
                    get_lsb(board->pieces[board->player == WHITE ? B_KING
                                                                 : W_KING]),
                    board->player)) {
                perft(board, depth - 1, nodes);
            }
            unmake_move(board, moves[i]);
        }
    }
}

/* Performance test for enumerating mostly legal moves to a certain depth */
static inline void speedy_perft(Board *board, int depth, U64 *nodes) {
    Move moves[MAX_MOVES];
    int count = generate_moves(board, moves);

    if (depth == 1) {
        *nodes += (U64)count;
    } else {
        for (int i = 0; i < count; i++) {
            make_move(board, moves[i]);
            if (!is_attacked(
                    board,
                    get_lsb(board->pieces[board->player == WHITE ? B_KING
                                                                 : W_KING]),
                    board->player)) {
                speedy_perft(board, depth - 1, nodes);
            }
            unmake_move(board, moves[i]);
        }
    }
}

/* Performance test for enumerating all pseudo legal moves to a certain depth */
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

/* Tests speed to loop through depth * 1 billion times */
static inline U64 loop_speed(int depth, U64 *nodes) {
    U64 billion = 1000000000;
    for (U64 i = 0; i < depth * billion; i++) {
        *nodes += 1;
    }

    return depth * billion;
}
