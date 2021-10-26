#include "board.h"
#include "constants.h"
#include "move.h"
#include "move_generation.h"

Board board;

/* Simple 2 player chess program */
static void two_player() {
    char move[5] = {0};
    int one, two, three, four, five;
    Move last;
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

        switch (tolower(move[4])) {
        case 'n':
            five = PROMOTION_N;
            break;
        case 'b':
            five = PROMOTION_B;
            break;
        case 'r':
            five = PROMOTION_R;
            break;
        case 'q':
            five = PROMOTION_Q;
            break;
        default:
            five = 0;
            break;
        }

        if (undo && !strcmp(move, "undo")) {
            unmove_piece(last);
            undo = false;
            printf("\n");
            print_board();
        } else if (!strcmp(move, "0-0")) {
            if ((board.player == WHITE && !move_legal(&last, E1, G1, false)) ||
                (board.player == BLACK && !move_legal(&last, E8, G8, false))) {
                undo = true;
                printf("\n");
                print_board();
            }
        } else if (!strcmp(move, "0-0-0")) {
            if ((board.player == WHITE && !move_legal(&last, E1, C1, false)) ||
                (board.player == BLACK && !move_legal(&last, E8, C8, false))) {
                undo = true;
                printf("\n");
                print_board();
            }
        } else if (one >= 0 && two >= 0 && three >= 0 && four >= 0 &&
                   one <= 7 && two <= 7 && three <= 7 && four <= 7) {
            Fast start = one + two * 16, end = three + four * 16;
            Fast square = board.player == WHITE ? 6 : 1;

            if (two == square && board.pieces[start] == PAWN) {
                if (five && !move_legal(&last, start, end, five)) {
                    undo = true;
                    printf("\n");
                    print_board();
                }
            } else if (!move_legal(&last, start, end, false)) {
                undo = true;
                printf("\n");
                print_board();
            }
        }
    }
}

/* Test file for code */
int main() {
    two_player();

    return 0;
}