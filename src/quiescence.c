#include "quiescence.h"
#include "attacks.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"

// Continue limited search until a quiet position is reached
int quiescence_search(Board *board, int alpha, int beta) {
    if (time_over) {
        return INVALID_SCORE;
    }

    log_increment(qnodes);

    // Lower bound of score
    int score = eval(board);
    if (score > alpha) {
        if (score >= beta) {
            return beta;
        }
        alpha = score;
    }

    // Search only captures and queen promotions
    Move moves[MAX_MOVES];
    int count = generate_quiescence_moves(board, moves);
    for (int i = 0; i < count; i++) {
        // Recursively search game tree
        make_move(board, moves[i]);

        // Removes illegal moves
        if (in_check(board, !board->player)) {
            unmake_move(board, moves[i]);
            continue;
        }

        score = -quiescence_search(board, -beta, -alpha);
        unmake_move(board, moves[i]);

        // Alpha cutoff
        if (score > alpha) {
            // Beta cutoff
            if (score >= beta) {
                return beta;
            }
            alpha = score;
        }
    }

    return alpha;
}
