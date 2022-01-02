#include "attacks.h"
#include "benchmark.h"
#include "bitboard.h"
#include "evaluation.h"
#include "game.h"

int main(void) {
    Board board;

    start_board(&board);
    init_evaluation();
    init_attacks();

    int seconds = 10;
    start_singleplayer(&board, WHITE, seconds);

    return SUCCESS;
}
