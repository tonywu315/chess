#include "board.h"
#include "constants.h"
#include "move.h"
#include "move_generation.h"
#include <stdio.h>

Board board;

/* Test file for code */
int main() {
    Move moves[MAX_MOVES];
    int count;

    start_board();

    if (player_move_piece(create_move(F2, F3, NORMAL))) printf("FAILURE1\n");
    if (player_move_piece(create_move(E7, E5, NORMAL))) printf("FAILURE2\n");
    if (player_move_piece(create_move(D2, D3, NORMAL))) printf("FAILURE3\n");
    if (player_move_piece(create_move(D7, D5, NORMAL))) printf("FAILURE4\n");
    if (player_move_piece(create_move(G2, G4, NORMAL))) printf("FAILURE5\n");
    if (player_move_piece(create_move(D8, H4, NORMAL))) printf("FAILURE6\n");
    if (player_move_piece(create_move(C1, E3, NORMAL))) printf("SUCCESS\n");
    if (player_move_piece(create_move(E1, D2, NORMAL))) printf("FAILURE7\n");
    if (player_move_piece(create_move(A7, A6, NORMAL))) printf("FAILURE8\n");

    print_board();

    count = generate_moves(moves);

    printf("Number of moves: %d\n", count);

    return 0;
}