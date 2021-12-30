#include "bitboard.h"
#include "move_generation.h"
#include "move.h"

/* Start game */
int main(void) {
    Board board;

    start_board(&board);
    init_attacks();

    return SUCCESS;
}
