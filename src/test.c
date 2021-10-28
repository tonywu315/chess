#include "board.h"
#include "constants.h"
#include "move.h"
#include "move_generation.h"

typedef unsigned long long u64;

Board board;
Move game_moves[MAX_GAME_LENTH];
int root_pos;
int search_pos;

static void two_player();
static void benchmark();
static u64 perft(int depth);
static u64 pseudo_perft(int depth);

/* Test file for code */
int main() {
    root_pos = 0, search_pos = 0;

    benchmark(6);

    return SUCCESS;
}

/* Simple 2 player chess program */
static void two_player() {
    char move[5] = {0};
    int one, two, three, four, five;

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

        if (!strcmp(move, "undo") && search_pos) {
            unmove_piece();
            print_board();
        } else if (!strcmp(move, "0-0")) {
            if ((board.player == WHITE && !move_legal(E1, G1, false)) ||
                (board.player == BLACK && !move_legal(E8, G8, false))) {
                print_board();
            }
        } else if (!strcmp(move, "0-0-0")) {
            if ((board.player == WHITE && !move_legal(E1, C1, false)) ||
                (board.player == BLACK && !move_legal(E8, C8, false))) {
                print_board();
            }
        } else if (one >= 0 && two >= 0 && three >= 0 && four >= 0 &&
                   one <= 7 && two <= 7 && three <= 7 && four <= 7) {
            Fast start = one + two * 16, end = three + four * 16;
            Fast square = board.player == WHITE ? 6 : 1;

            if (two == square && board.pieces[start] == PAWN) {
                if (five && !move_legal(start, end, five)) {
                    print_board();
                }
            } else if (!move_legal(start, end, false)) {
                print_board();
            }
        }
    }
}

/* Computes time to complete task */
static void benchmark(int depth) {
    int nodes;
    double time;
    clock_t begin_time, end_time;

    start_board();

    begin_time = clock();
    nodes = pseudo_perft(depth);
    end_time = clock();
    time = (double)(end_time - begin_time) / CLOCKS_PER_SEC;

    printf("Depth %d, Nodes: %d\n", depth, nodes);
    printf("Time: %lf seconds, MNPS: %.3f\n\n", time, nodes / (time * 1000000));
}

/* Performance test for enumerating all moves to a certain depth */
static u64 perft(int depth) {
    Move move_list[MAX_MOVES];
    int count, i;
    u64 nodes = 0;

    if (depth == 0) {
        return 1ULL;
    }

    count = generate_moves(move_list);

    for (i = 0; i < count; i++) {
        move_piece(&move_list[i]);
        if (!in_check()) {
            nodes += perft(depth - 1);
        }
        unmove_piece();
    }
    return nodes;
}

/* Performance test for enumerating all pseudo legal moves to a certain depth */
static u64 pseudo_perft(int depth) {
    Move move_list[MAX_MOVES];
    int count, i;
    u64 nodes = 0;

    count = generate_moves(move_list);

    if (depth == 1) {
        return (u64)count;
    }

    for (i = 0; i < count; i++) {
        move_piece(&move_list[i]);
        nodes += pseudo_perft(depth - 1);
        unmove_piece();
    }
    return nodes;
}