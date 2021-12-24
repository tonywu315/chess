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

    Bitboard test = UINT64_C(0);
    for (U8 square = A1; square <= H8; square++) {
        if (is_attacked(board, square, WHITE)) {
            set_bit(&test, square);
        }
    }

    print_bitboard(test);

    return SUCCESS;
}
