#include "board.h"
#include "constants.h"
#include "move.h"
#include <stdio.h>

Board board;

/* Test file for code */
int main() {
    Move moves[MAX_MOVES];
    int count;

    start_board();
    move_piece(E2, E4);
    move_piece(D8, D3);
    move_piece(F1,F1);
    move_piece(G1,G1);
    move_piece(D1,E2);
    move_piece(B1,B1);
    move_piece(C1,C1);
    move_piece(D1,D1);
    print_board();

    count = generate_moves(moves);

    return 0;
}