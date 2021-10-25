#include "board.h"
#include "constants.h"
#include "move.h"
#include <stdio.h>

/* Test file for code */
int main() {
    Board board;
    Move moves[MAX_MOVES];
    int count;

    start_board(&board);
    move_piece(&board, E2, E4);
    move_piece(&board, D8, D3);
    print_board(&board);

    count = generate_moves(&board, moves);

    printf("%d\n", count);

    return 0;
}