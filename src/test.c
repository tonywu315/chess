#include "benchmark.h"
#include "board.h"
#include "move.h"
#include "move_generation.h"
#include "search.h"
#include "evaluation.h"

Board board;
Move game_moves[MAX_GAME_LENTH];
int root_pos;
int search_pos;

/* Makes computer move if not single player and prints board */
static void next(bool singleplayer) {
    int score;

    if (singleplayer) {
        score = eval();
    } else {
        Line mainline;
        score = alpha_beta(-INT_MAX, INT_MAX, 6, &mainline);
        move_piece(&mainline.moves[0]);
    }

    print_board(-score);
}

/* Start chess engine */
void start_game(bool singleplayer) {
    char move[5] = {0};
    int one, two, three, four, five;

    start_board();

    printf("=== Chess Program ===\n");
    print_board(0);

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

        if (!strcmp(move, "undo") && search_pos) {
            unmove_piece();
            next(singleplayer);
        } else if (!strcmp(move, "0-0")) {
            if ((board.player == WHITE && !move_legal(E1, G1, false)) ||
                (board.player == BLACK && !move_legal(E8, G8, false))) {
                next(singleplayer);
            }
        } else if (!strcmp(move, "0-0-0")) {
            if ((board.player == WHITE && !move_legal(E1, C1, false)) ||
                (board.player == BLACK && !move_legal(E8, C8, false))) {
                next(singleplayer);
            }
        } else if (one >= 0 && two >= 0 && three >= 0 && four >= 0 &&
                   one <= 7 && two <= 7 && three <= 7 && four <= 7) {
            Fast start = one + two * 16, end = three + four * 16;
            Fast square = board.player == WHITE ? 6 : 1;

            if (two == square && board.pieces[start] == PAWN) {
                if (five && !move_legal(start, end, five)) {
                    next(singleplayer);
                }
            } else if (!move_legal(start, end, false)) {
                next(singleplayer);
            }
        }
    }
}

/* Test file for code */
int main() {
    load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    print_board(eval());

    return SUCCESS;
}