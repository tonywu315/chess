#include "search.h"
#include "attacks.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "move_order.h"
#include "quiescence.h"
#include "transposition.h"

Info info;
// Replay replay;
bool time_over;

static int search(Board *board, Stack *stack, int alpha, int beta, int depth);
static inline bool is_repetition(Board *board);
static inline void update_pv(Stack *stack, Move move);

void start_search(Board *board, Parameter parameters) {
    Stack stack[MAX_DEPTH] = {0};
    Move best_move = NULL_MOVE, ponder_move = NULL_MOVE;
    int best_score = INVALID_SCORE;
    U64 end_time = parameters.start_time;

    double depth_time = 0;
    U64 final_nodes = 0;

    for (int ply = 0; ply < MAX_DEPTH; ply++) {
        stack[ply].ply = ply;
    }

    int max_depth = parameters.max_depth ? parameters.max_depth : MAX_DEPTH;
    for (info.depth = 1; info.depth <= 10; info.depth++) {
        // TODO: test if clearing killer tables improves search

        depth_time = get_time();
        info.nodes = 0;
        info.hnodes = 0;
        info.qnodes = 0;
        info.tt_hits = 0;
        info.tt_cuts = 0;

        int score = search(board, stack, -INFINITY, INFINITY, info.depth);

        // Stop searching if time is over and discard unfinished score
        if (is_time_over()) {
            info.depth--;
            break;
        }

        // Save principal variation moves to the transposition table
        set_pv_moves(board, stack, score);

        best_move = stack[0].pv_moves[0];
        ponder_move = stack[0].pv_moves[1];
        best_score = board->player == WHITE ? score : -score;

        if (DEBUG_FLAG) {
            final_nodes = info.hnodes;
            end_time = get_time();
            double time_taken =
                (double)(end_time - depth_time) / 1000;

            printf("\nDepth: %d\n", info.depth);

            printf("Evaluation: %d\n", best_score);
            printf("Best moves:");
            for (int i = 0; i < stack[0].pv_length; i++) {
                printf(" %s%s",
                       get_coordinates(get_move_start(stack[0].pv_moves[i])),
                       get_coordinates(get_move_end(stack[0].pv_moves[i])));
            }

            printf("\nNodes: %lld\n", info.nodes + info.qnodes);
            printf(" - Interior:  %lld\n", info.nodes);
            printf(" - Horizon:   %lld\n", info.hnodes);
            printf(" - Quiescent: %lld\n", info.qnodes - info.hnodes);
            printf("TT Hits: %lld (%.2lf%%)\n", info.tt_hits,
                   100.0 * info.tt_hits / info.nodes);
            printf(" - TT Cuts: %lld (%.2lf%%)\n", info.tt_cuts,
                   100.0 * info.tt_cuts / info.nodes);
            printf("Time: %.3lf seconds, KNPS: %.0f\n", time_taken,
                   (info.nodes + info.qnodes) / (time_taken * 1000));
        }
    }

    if (DEBUG_FLAG) {
        printf("\nTotal time: %.3lf seconds\n",
               (double)(end_time - parameters.start_time) / 1000);
        printf("Branching factor: %.2lf\n", pow(final_nodes, 1.0 / info.depth));
    }

    printf("bestmove %s%s ponder %s%s\n",
           get_coordinates(get_move_start(best_move)),
           get_coordinates(get_move_end(best_move)),
           get_coordinates(get_move_start(ponder_move)),
           get_coordinates(get_move_end(ponder_move)));
}

// Search board for best move
static int search(Board *board, Stack *stack, int alpha, int beta, int depth) {
    int tt_flag = UPPER_BOUND;
    int ply = stack->ply;
    bool root_node = ply == 0;

    // printf("hash: %llx\n", board->hash);
    // printf("actual hash: %llx\n", get_hash(board));

    // Check for repetition
    if (is_repetition(board)) {
        return DRAW_SCORE;
    }

    if (!root_node) {
        // Time over
        if (is_time_over()) {
            return INVALID_SCORE;
        }

        // Mate distance pruning
        alpha = MAX(alpha, -INFINITY + ply);
        beta = MIN(beta, INFINITY - ply - 1);
        if (ply && alpha >= beta) {
            return alpha;
        }
    }

    // Check extension
    bool check = in_check(board, board->player);
    if (check) {
        depth += 1;
    }

    // Quiescence search at leaf nodes
    if (depth == 0) {
        increment(info.hnodes); // FIXME:
        stack->pv_length = 0;
        return quiescence_search(board, alpha, beta);
    }

    increment(info.nodes); // FIXME:

    // Check if position is in transposition table
    Move tt_move = NULL_MOVE;
    int score =
        get_transposition(board->hash, alpha, beta, ply, depth, &tt_move);

    if (score != NO_TT_HIT) {
        increment(info.tt_hits); // FIXME:

        if (!root_node && score != TT_HIT) {
            // TODO: add conditional after implementing PVS
            // Return score in non-pv nodes
            // if (alpha + 1 == beta) {}
            increment(info.tt_cuts); // FIXME:
            stack->pv_length = 0;
            return score;
        }
    }

    // Generate pseudo legal moves and score them
    Move moves[MAX_MOVES], best_move = NULL_MOVE;
    MoveList move_list[MAX_MOVES];
    int count = generate_moves(board, moves), moves_count = 0;
    score_moves(board, stack, moves, move_list, tt_move, count);

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
        score = -search(board, stack + 1, -beta, -alpha, depth - 1);

        unmake_move(board, move);

        if (is_time_over()) {
            return INVALID_SCORE;
        }

        // Alpha cutoff
        if (score > alpha) {
            best_move = move;

            // Update principal variation
            update_pv(stack, best_move);

            // Beta cutoff
            if (score >= beta) {
                // Store quiet moves that cause beta cutoffs
                if (board->board[get_move_end(move)] == NO_PIECE &&
                    get_move_flag(move) == NORMAL_MOVE &&
                    stack->killers[0] != move) {
                    stack->killers[1] = stack->killers[0];
                    stack->killers[0] = move;
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
        alpha = check ? -INFINITY + ply : DRAW_SCORE;
    }

    // Save position to transposition table
    set_transposition(board->hash, alpha, tt_flag, ply, depth, best_move);

    return alpha;
}

// Check if position has occurred before
static inline bool is_repetition(Board *board) {
    int draw_ply = board->state[board->ply].draw_ply;

    for (int i = board->ply - 4; i >= board->ply - draw_ply; i -= 2) {
        if (board->hash == board->hashes[i]) {
            return true;
        }
    }
    return false;
}

// Update principal variation
static inline void update_pv(Stack *stack, Move move) {
    *stack->pv_moves = move;
    memcpy(stack->pv_moves + 1, (stack + 1)->pv_moves,
           (stack + 1)->pv_length * sizeof(Move));
    stack->pv_length = (stack + 1)->pv_length + 1;
}
