#include "board.h"
#include "constants.h"
#include "move.h"
#include "move_generation.h"

Board board;

/* Simple 2 player chess program */
static void two_player() {
    char move[5] = {0};
    int one, two, three, four;
    Move previous;
    bool undo = false;

    start_board();

    printf("=== Chess Program ===\n");
    print_board();

    while (true) {
        printf("Move: ");
        scanf("%5s", move);

        one = tolower(move[0]) - 'a';
        two = move[1] - '1';
        three = tolower(move[2]) - 'a';
        four = move[3] - '1';

        if (undo && !strcmp(move, "undo")) {
            unmove_piece(previous);
            undo = false;
            printf("\n");
            print_board();
        } else if (!strcmp(move, "0-0")) {
            if ((board.player == WHITE && !move_legal(&previous, E1, G1)) ||
                (board.player == BLACK && !move_legal(&previous, E8, G8))) {
                undo = true;
                printf("\n");
                print_board();
            }
        } else if (!strcmp(move, "0-0-0")) {
            if ((board.player == WHITE && !move_legal(&previous, E1, C1)) ||
                (board.player == BLACK && !move_legal(&previous, E8, C8))) {
                undo = true;
                printf("\n");
                print_board();
            }
        } else if (one >= 0 && two >= 0 && three >= 0 && four >= 0 &&
                   one <= 8 && two <= 8 && three <= 8 && four <= 8 &&
                   !move_legal(&previous, one + two * 16, three + four * 16)) {
            undo = true;
            printf("\n");
            print_board();
        }
    }
}

/* Test file for code */
int main() {
    two_player();

    return 0;
}