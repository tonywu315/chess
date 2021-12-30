#include "attacks.h"
#include "bitboard.h"

/* Start game */
int main(void) {
    Board board;

    start_board(&board);
    init_attacks();

    print_board(board, 0);

    return SUCCESS;
}
