#include "attacks.h"
#include "benchmark.h"
#include "bitboard.h"
#include "evaluation.h"
#include "game.h"
#include "transposition.h"

int main(int argc, char **argv) {
    Board board;
    int seconds = 10;

    init_attacks();
    init_evaluation();
    init_transposition(512);

    start_board(&board);

    if (argc >= 2) {
        seconds = atoi(argv[1]);

        if (argc >= 3 && !strcmp(argv[2], "benchmark")) {
            benchmark(&board, 6);
        }
    }

    start_singleplayer(&board, true, seconds);

    if (transposition) {
        free(transposition);
    }

    return SUCCESS;
}
