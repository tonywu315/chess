#include "attacks.h"
#include "bitboard.h"
#include "evaluation.h"
#include "game.h"

#include "benchmark.h"

int main(int argc, char **argv) {
    Board board;
    int seconds = 10;

    init_attacks();
    init_evaluation();
    init_hash_keys();
    // init_transposition(256);

    start_board(&board);

    if (argc == 2) {
        seconds = atoi(argv[1]);
    }

    benchmark(&board, 6);

    start_singleplayer(&board, WHITE, seconds);

    return SUCCESS;
}
