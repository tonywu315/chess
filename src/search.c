#include "search.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"

/* Alpha beta pruning */
int alpha_beta(int alpha, int beta, int ply, int depth, Line *mainline) {
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

    /* Iterate over all moves */
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

        /* Calculate score of opponent */
        score = -alpha_beta(-beta, -alpha, ply + 1, depth - 1, &line);
        unmove_piece();

        /* Alpha cutoff */
        if (score > alpha) {
            mainline->moves[0] = move_list[i];
            memcpy(mainline->moves + 1, line.moves, line.length * sizeof(Move));
            mainline->length = line.length + 1;

            /* Beta cutoff */
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
        alpha = check ? -INT_MAX + ply : 0;
    }

    return alpha;
}
