#include "attacks.h"
#include "bitboard.h"
#include "evaluation.h"
#include "game.h"

int main(int argc, char **argv) {
    Board board;
    int seconds = 10;

    start_board(&board);
    init_evaluation();
    init_attacks();

    if (argc == 2) {
        seconds = atoi(argv[1]);
    }

    start_singleplayer(&board, WHITE, seconds);

    return SUCCESS;
}
