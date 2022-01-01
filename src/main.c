#include "attacks.h"
#include "bitboard.h"
#include "evaluation.h"
#include "game.h"

#include "benchmark.h"

/* Start game */
int main(void) {
    Board board;

    start_board(&board);
    init_evaluation();
    init_attacks();

    start_singleplayer(&board, WHITE, 6);

    return SUCCESS;
}
