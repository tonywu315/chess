#include "search.h"
#include "attacks.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "quiescence.h"
#include "transposition.h"

U64 nodes;
U64 hnodes;
U64 qnodes;
U64 ttnodes;
bool time_over;

static int search(Board *board, int alpha, int beta, int ply, int depth,
                  Line *mainline);
static void *chess_clock(void *time);

// Search position for best move within time limit
int search_position(Board *board, Move *move, int time) {
    pthread_t tid;
    clock_t depth_time, start_time = clock();
    Line mainline = {0, {0}};
    Move best_move = 0;
    int best_score = 0, score = 0;
    int alpha = -INFINITY, beta = INFINITY;

    Move moves[MAX_MOVES];
    int move_count = generate_legal_moves(board, moves);

    // Start timer
    time_over = false;
    pthread_create(&tid, NULL, chess_clock, (void *)&time);

    // Iterative deepening
    for (int depth = 1;; depth++) {
        if (LOG_FLAG) {
            depth_time = clock();
            nodes = 0;
            hnodes = 0;
            qnodes = 0;
            ttnodes = 0;
        }

        score = search(board, alpha, beta, 0, depth, &mainline);

        // Stop searching if time is over and discard unfinished score
        if (time_over) {
            break;
        }

        best_move = mainline.moves[0];
        best_score = board->player == WHITE ? score : -score;

        if (LOG_FLAG) {
            clock_t now = clock();
            double time_taken = (double)(now - depth_time) / CLOCKS_PER_SEC;
            double total_time = (double)(now - start_time) / CLOCKS_PER_SEC;

            printf("\nDepth: %d\n", depth);

            printf("Evaluation: %d\n", best_score);
            printf("Best moves:");
            for (int i = 0; i < mainline.length; i++) {
                printf(" %s%s",
                       get_coordinates(get_move_start(mainline.moves[i])),
                       get_coordinates(get_move_end(mainline.moves[i])));
            }

            printf("\nNodes: %" PRId64 "\n", nodes + qnodes);
            printf(" - Interior:  %" PRId64 "\n", nodes);
            printf(" - Horizon:   %" PRId64 "\n", hnodes);
            printf(" - Quiescent: %" PRId64 "\n", qnodes - hnodes);
            printf("TTNodes: %" PRId64 " (%.2lf%%)\n", ttnodes,
                   100.0 * ttnodes / nodes);

            printf("Time: %.3lf seconds, KNPS: %.0f\n", time_taken,
                   (nodes + qnodes) / (time_taken * 1000));
            printf("Total time: %.3lf seconds\n", total_time);
        }

        // Stop searching if there is 1 legal move or max depth is reached
        if ((move_count == 1) || depth == MAX_DEPTH) {
            pthread_cancel(tid);
            break;
        }
    }

    // Free resources
    pthread_join(tid, NULL);

    // Set move to mainline and return score
    *move = best_move;
    return best_score;
}

// Alpha beta algorithm
static int search(Board *board, int alpha, int beta, int ply, int depth,
                  Line *mainline) {
    Move moves[MAX_MOVES], tt_move = NULLMOVE, best_move = NULLMOVE;
    Line line = {0, {0}};
    uint8_t tt_flag = UPPER_BOUND;
    int score;

    // Mate distance pruning
    if (alpha < -INFINITY + ply) {
        alpha = -INFINITY + ply;
    }
    if (beta > INFINITY - ply - 1) {
        beta = INFINITY - ply - 1;
    }
    if (ply && alpha >= beta) {
        return alpha;
    }

    // Check extension
    bool check = in_check(board, board->player);
    if (check) {
        depth += 1;
    }

    // Quiescence search at leaf nodes
    if (depth == 0) {
        mainline->length = 0;
        log_run(hnodes++;);
        return quiescence_search(board, alpha, beta);
    }

    log_run(nodes++);

    // Check if position is in transposition table
    if (ply) {
        score =
            get_transposition(board->hash, alpha, beta, ply, depth, &tt_move);

        if (score != INVALID_SCORE) {
            log_run(ttnodes++);

            // Return score in non-pv nodes or on exact hash hits
            if (alpha + 1 >= beta || (score > alpha && score < beta)) {
                return score;
            }
        }
    }

    // Iterate over all pseudo legal moves
    int count = generate_moves(board, moves), moves_count = 0;
    for (int i = 0; i < count; i++) {
        if (time_over) {
            return INVALID_SCORE;
        }

        make_move(board, moves[i]);

        // Removes illegal moves
        if (in_check(board, !board->player)) {
            unmake_move(board, moves[i]);
            continue;
        }

        moves_count++;

        // Recursively search game tree
        score = -search(board, -beta, -alpha, ply + 1, depth - 1, &line);
        unmake_move(board, moves[i]);

        // Alpha cutoff
        if (score > alpha) {
            best_move = moves[i];

            // Update mainline
            mainline->moves[0] = best_move;
            memcpy(mainline->moves + 1, line.moves, line.length * sizeof(Move));
            mainline->length = line.length + 1;

            // Beta cutoff
            if (score >= beta) {
                alpha = beta;
                tt_flag = LOWER_BOUND;
                break;
            }

            alpha = score;
            tt_flag = EXACT_BOUND;
        }
    }

    // Checkmate and stalemate
    if (moves_count == 0) {
        alpha = check ? -INFINITY + ply : 0;
    }

    // Save position to transposition table
    set_transposition(board->hash, alpha, tt_flag, ply, depth, best_move);

    return alpha;
}

// Sleep time seconds
static void *chess_clock(void *time) {
    sleep(*(int *)time);

    time_over = true;

    return NULL;
}
