#include "search.h"
#include "attacks.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "move_order.h"
#include "quiescence.h"
#include "transposition.h"

bool time_over;

clock_t depth_time, start_time, end_time;
Search info = {0};
Replay replay = {0};

Move killers[MAX_DEPTH][2];

static int search(Board *board, int alpha, int beta, int ply, int depth,
                  Line *mainline);
static void *chess_clock(void *time);

// Search position for best move within time limit
int search_position(Board *board, Move *move, int time) {
    pthread_t tid;
    Line mainline = {{0}, 0};
    Move best_move = 0;
    int best_score = 0, score = 0;
    U64 final_nodes = 0;

    Move moves[MAX_MOVES];
    int move_count = generate_legal_moves(board, moves);

    // Start timer
    time_over = false;
    pthread_create(&tid, NULL, chess_clock, (void *)&time);

    if (DEBUG_FLAG) {
        start_time = end_time = clock();
    }

    // Iterative deepening
    for (info.depth = 1;; info.depth++) {
        if (DEBUG_FLAG) {
            depth_time = clock();
            info.nodes = 0;
            info.hnodes = 0;
            info.qnodes = 0;
            info.tt_hits = 0;
            info.tt_cuts = 0;
        }

        score = search(board, -INFINITY, INFINITY, 0, info.depth, &mainline);

        // Reset killers table
        memset(killers, 0, sizeof(killers));

        // Stop searching if time is over and discard unfinished score
        if (is_time_over()) {
            // Save partial search information for debugging
            if (DEBUG_FLAG) {
                replay.ply[game_ply].search.depth = info.depth;
                replay.ply[game_ply].search.nodes = info.nodes;
                replay.ply[game_ply].search.qnodes = info.qnodes;
            }

            // Decrement depth because it was not fully searched
            info.depth--;
            break;
        }

        // Save principal variation moves to the transposition table
        set_pv_moves(board, &mainline, score);

        best_move = mainline.moves[0];
        best_score = board->player == WHITE ? score : -score;

        if (DEBUG_FLAG) {
            final_nodes = info.hnodes;
            end_time = clock();
            double time_taken =
                (double)(end_time - depth_time) / CLOCKS_PER_SEC;

            printf("\nDepth: %d\n", info.depth);

            printf("Evaluation: %d\n", best_score);
            printf("Best moves:");
            for (int i = 0; i < mainline.length; i++) {
                printf(" %s%s",
                       get_coordinates(get_move_start(mainline.moves[i])),
                       get_coordinates(get_move_end(mainline.moves[i])));
            }

            printf("\nNodes: %" PRId64 "\n", info.nodes + info.qnodes);
            printf(" - Interior:  %" PRId64 "\n", info.nodes);
            printf(" - Horizon:   %" PRId64 "\n", info.hnodes);
            printf(" - Quiescent: %" PRId64 "\n", info.qnodes - info.hnodes);
            printf("TT Hits: %" PRId64 " (%.2lf%%)\n", info.tt_hits,
                   100.0 * info.tt_hits / info.nodes);
            printf(" - TT Cuts: %" PRId64 " (%.2lf%%)\n", info.tt_cuts,
                   100.0 * info.tt_cuts / info.nodes);
            printf("Time: %.3lf seconds, KNPS: %.0f\n", time_taken,
                   (info.nodes + info.qnodes) / (time_taken * 1000));
        }

        // Stop searching if there is 1 legal move or max depth is reached
        if ((move_count == 1) || info.depth == MAX_DEPTH) {
            pthread_cancel(tid);
            break;
        }
    }

    if (DEBUG_FLAG) {
        printf("\nTotal time: %.3lf seconds\n",
               (double)(end_time - start_time) / CLOCKS_PER_SEC);
        printf("Branching factor: %.2lf\n", pow(final_nodes, 1.0 / info.depth));
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
    Move moves[MAX_MOVES], tt_move = 0, best_move = 0;
    MoveList move_list[MAX_MOVES];
    Line line = {{0}, 0};
    uint8_t tt_flag = UPPER_BOUND;

    // Time over
    if (is_time_over()) {
        return INVALID_SCORE;
    }

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
        increment(info.hnodes);
        mainline->length = 0;
        return quiescence_search(board, alpha, beta);
    }

    increment(info.nodes);

    // Check if position is in transposition table
    int score =
        get_transposition(board->hash, alpha, beta, ply, depth, &tt_move);

    if (score != NO_TT_HIT) {
        increment(info.tt_hits);

        if (ply && score != TT_HIT) {
            // TODO: add conditional after implementing PVS
            // Return score in non-pv nodes
            // if (alpha + 1 == beta) {}
            increment(info.tt_cuts);
            mainline->length = 0;
            return score;
        }
    }

    // Generate pseudo legal moves and score them
    int count = generate_moves(board, moves), moves_count = 0;
    score_moves(board, moves, move_list, tt_move, ply, count);

    // Iterate over moves
    for (int i = 0; i < count; i++) {
        // Move next best move to the front
        Move move = sort_moves(move_list, count, i);

        make_move(board, move);

        // Remove illegal moves
        if (in_check(board, !board->player)) {
            unmake_move(board, move);
            continue;
        }

        moves_count++;

        // Recursively search game tree
        score = -search(board, -beta, -alpha, ply + 1, depth - 1, &line);

        unmake_move(board, move);

        // Alpha cutoff
        if (score > alpha) {
            best_move = move;

            // Update mainline
            mainline->moves[0] = best_move;
            memcpy(mainline->moves + 1, line.moves, line.length * sizeof(Move));
            mainline->length = line.length + 1;

            // Beta cutoff
            if (score >= beta) {
                // Store killer moves that cause cutoffs
                if (board->board[get_move_end(move)] == NO_PIECE &&
                    get_move_flag(move) == NORMAL_MOVE &&
                    killers[ply][0] != move) {
                    killers[ply][1] = killers[ply][0];
                    killers[ply][0] = move;
                }

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
