#include "search.h"
#include "board.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"

/* Alpha beta pruning */
int alpha_beta(int alpha, int beta, int depth, Line *mainline) {
    Move move_list[MAX_MOVES];
    Line line;
    bool check = in_check(board.player);

    /* Check extension */
    if (check) {
        depth++;
    }

    /* TODO: add quiescence search */
    if (depth == 0) {
        mainline->length = 0;
        return eval();
    }

    int count = generate_moves(move_list), moves_count = 0;
    for (int i = 0; i < count; i++) {
        int score;
        move_piece(&move_list[i]);

        /* Removes illegal moves */
        if (in_check(3 - board.player)) {
            unmove_piece();
            continue;
        }

        moves_count++;

        score = -alpha_beta(-beta, -alpha, depth - 1, &line);
        unmove_piece();

        if (score > alpha) {
            mainline->moves[0] = move_list[i];
            memcpy(mainline->moves + 1, line.moves, line.length * sizeof(Move));
            mainline->length = line.length + 1;

            if (score >= beta) {
                alpha = beta;
                break;
            } else {
                alpha = score;
            }
        }
    }

    /* Checkmate and stalemate */
    if (moves_count == 0) {
        alpha = check ? -INT_MAX : 0;
        printf("reached\n");
    }

    return alpha;
}
