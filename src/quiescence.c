#include "quiescence.h"
#include "attacks.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "move_order.h"

// Continue limited search until a quiet position is reached
int quiescence_search(Board *board, int alpha, int beta) {
    Move moves[MAX_MOVES];
    MoveList move_list[MAX_MOVES];

    if (time_over) {
        return INVALID_SCORE;
    }

    info.nodes++;

    // Lower bound of score
    int score = eval(board);
    if (score > alpha) {
        if (score >= beta) {
            return beta;
        }
        alpha = score;
    }

    // Search only captures and queen promotions
    int count = generate_quiescence_moves(board, moves);
    score_quiescence_moves(board, moves, move_list, count);

    for (int i = 0; i < count; i++) {
        Move move = sort_moves(move_list, count, i);

        make_move(board, move);

        // Remove illegal moves
        if (in_check(board, !board->player)) {
            unmake_move(board, move);
            continue;
        }

        // Recursively search game tree
        score = -quiescence_search(board, -beta, -alpha);
        unmake_move(board, move);

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
