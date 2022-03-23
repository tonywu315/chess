#include "quiescence.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"

// Continue limited search until a quiet position is reached
int quiescence_search(Board *board, int alpha, int beta) {
    if (LOG_FLAG) {
        nodes_searched++;
    }

    int score = eval(board);

    if (score >= beta) {
        return beta;
    }
    if (score > alpha) {
        alpha = score;
    }

    // Search only captures and queen promotions
    Move moves[MAX_MOVES];
    int count = generate_quiescence_moves(board, moves);
    for (int i = 0; i < count; i++) {
        if (time_over) {
            return INVALID_SCORE;
        }

        if (board->board[get_move_end(moves[i])] ==
            make_piece(PAWN, board->player)) {
            return INFINITY;
        }

        // Recursively search game tree
        make_move(board, moves[i]);
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
