#include "attacks.h"
#include "benchmark.h"
#include "bitboard.h"
#include "move_generation.h"

/* Start game */
int main(void) {
    Board board;

    start_board(&board);
    init_attacks();

    load_fen(
        &board,
        "r3k2r/1P2pnb1/b1p3p1/q2p2Pp/Q3n3/N4N2/PB1P1PBP/R3K2R w KQkq h6 0 20");

    print_board(&board, 0);

    benchmark(&board, 1);

    Move moves[MAX_MOVES];
    int count = generate_moves(&board, moves);

    for (int i = 0; i < count; i++) {
        printf("%s %s\n", get_coordinates(get_move_start(moves[i])),
               get_coordinates(get_move_end(moves[i])));
    }

    return SUCCESS;
}
