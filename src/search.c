#include "search.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "attacks.h"

/* Alpha beta pruning */
int search(Board *board, int alpha, int beta, int ply, int depth,
           Line *mainline) {
    Move moves[MAX_MOVES];
    Line line;

    bool check = is_attacked(
        board, get_lsb(board->pieces[make_piece(KING, board->player)]),
        !board->player);

    /* Check extension */
    if (check) {
        depth++;
    }

    /* TODO: add quiescence search */
    if (depth == 0) {
        mainline->length = 0;
        return eval(board);
    }

    /* Iterate over all moves */
    int count = generate_moves(board, moves), moves_count = 0;
    for (int i = 0; i < count; i++) {
        int score;
        make_move(board, moves[i]);

        /* Removes illegal moves */
        if (is_attacked(
                board, get_lsb(board->pieces[make_piece(KING, !board->player)]),
                board->player)) {
            unmake_move(board, moves[i]);
            continue;
        }

        moves_count++;

        /* Calculate score of opponent */
        score = -search(board, -beta, -alpha, ply + 1, depth - 1, &line);
        unmake_move(board, moves[i]);

        /* Alpha cutoff */
        if (score > alpha) {
            mainline->moves[0] = moves[i];
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
