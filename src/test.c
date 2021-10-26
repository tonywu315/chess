#include "board.h"
#include "move.h"
#include "move_generation.h"
#include <stdio.h>

Board board;

/* Test file for code */
int main() {
    start_board();
    print_board();

    Move moves[MAX_MOVES];
    int count = generate_moves(moves);

    printf("Number of moves: %d\n", count);

    return 0;
}