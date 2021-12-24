#include "bitboard.h"
#include "bitboard_move_generation.h"
#include "game.h"

OldBoard oldboard;
Move game_moves[MAX_MOVES];
int game_position;

/* Start game */
int main(void) {
    Board board;

    start_board(&board);
    init_attacks();

    print_board(board, 0);

    return SUCCESS;
}
