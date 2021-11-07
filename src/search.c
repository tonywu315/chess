#include "search.h"
#include "board.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"

/* Alpha beta pruning */
int alpha_beta(int alpha, int beta, int depth, Line *mainline) {
    Move move_list[MAX_MOVES];
    Line line;

    /* TODO: add quiescence search */
    if (depth == 0) {
        mainline->length = 0;
        return eval();
    }

    int count = generate_moves(move_list);
    for (int i = 0; i < count; i++) {
        int score;
        move_piece(&move_list[i]);

        /* Removes illegal moves */
        if (in_check()) {
            unmove_piece();
            continue;
        }

        score = -alpha_beta(-beta, -alpha, depth - 1, &line);
        unmove_piece();

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
            mainline->moves[0] = move_list[i];
            memcpy(mainline->moves + 1, line.moves, line.length * sizeof(Move));
            mainline->length = line.length + 1;
        }
    }

    return alpha;
}
