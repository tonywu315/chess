#include "search.h"
#include "attacks.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "move_order.h"
#include "quiescence.h"
#include "transposition.h"

Info info;
int game_ply;
bool time_over;

static int search(Board *board, Stack *stack, int alpha, int beta, int depth);
static inline bool is_repetition(Board *board);
static inline void update_pv(Stack *stack, Move move);
static inline bool check_time(Parameter *parameters, U64 elapsed_time,
                              int player);

void start_search(Board *board, Parameter parameters) {
    Move best_move = NULL_MOVE, ponder_move = NULL_MOVE;

    // Initialize stack
    Stack stack[MAX_PLY + 1] = {0};
    for (int ply = 0; ply <= MAX_PLY; ply++) {
        stack[ply].ply = ply;
    }

    // Iterative deepening
    int max_depth = parameters.max_depth ? parameters.max_depth : MAX_DEPTH;
    for (int depth = 1; depth <= max_depth; depth++) {
        // TODO: test if clearing killer tables improves search
        for (int ply = 0; ply <= info.seldepth; ply++) {
            stack[ply].killer_moves[0] = NULL_MOVE;
            stack[ply].killer_moves[1] = NULL_MOVE;
        }

        int score = search(board, stack, -INFINITY, INFINITY, depth);

        // Stop searching if time is over and discard unfinished score
        if (time_over) {
            time_over = false;
            break;
        }

        // Save principal variation moves to the transposition table
        set_pv_moves(board, stack, score);

        best_move = stack[0].pv_moves[0];
        ponder_move = stack[0].pv_moves[1];
        U64 elapsed_time = get_time() - parameters.start_time + 1;

        // Print info
        printf("info ");
        printf("depth %d ", depth);
        printf("seldepth %d ", info.seldepth);
        printf("time %lld ", elapsed_time);
        printf("nodes %lld ", info.nodes);
        printf("score cp %d ", score);
        printf("hashfull %d ", get_hashfull());
        printf("nps %.0lf ", (info.nodes * 1000.0) / elapsed_time);
        printf("pv");
        for (int i = 0; i < stack[0].pv_length; i++) {
            printf(" %s%s",
                   get_coordinates(get_move_start(stack[0].pv_moves[i])),
                   get_coordinates(get_move_end(stack[0].pv_moves[i])));
        }
        printf("\n");

        // printf(" string tt hits/cuts: (%.2lf%%, %.2lf%%)\n",
        //        100.0 * info.tt_hits / info.nodes,
        //        100.0 * info.tt_cuts / info.nodes);

        // Check if we still have time to search deeper
        if (check_time(&parameters, elapsed_time, board->player)) {
            break;
        }
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

    if (time_over) {
        return INVALID_SCORE;
    }

    info.seldepth = MAX(info.seldepth, ply);

    // Check for repetition
    if (is_repetition(board)) {
        return DRAW_SCORE;
    }

    // Mate distance pruning
    alpha = MAX(alpha, -INFINITY + ply);
    beta = MIN(beta, INFINITY - ply - 1);
    if (ply && alpha >= beta) {
        return alpha;
    }

    // Check extension
    bool check = in_check(board, board->player);
    if (check) {
        depth += 1;
    }

    // Quiescence search at leaf nodes
    if (depth == 0 || ply == MAX_PLY) {
        stack->pv_length = 0;
        return quiescence_search(board, alpha, beta);
    }

    info.nodes++;

    // Check if position is in transposition table
    Move tt_move = NULL_MOVE;
    int score =
        get_transposition(board->hash, alpha, beta, ply, depth, &tt_move);

    if (score != NO_TT_HIT) {
        info.tt_hits++;

        if (!root_node && score != TT_HIT) {
            // TODO: add conditional after implementing PVS
            // Return score in non-pv nodes
            // if (alpha + 1 == beta) {}
            info.tt_cuts++;
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

        if (time_over) {
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
                    stack->killer_moves[0] != move) {
                    stack->killer_moves[1] = stack->killer_moves[0];
                    stack->killer_moves[0] = move;
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

// Check if there is enough time to search deeper
static inline bool check_time(Parameter *parameters, U64 elapsed_time,
                              int player) {
    // Do not stop if infinite time
    if (parameters->infinite ||
        (!parameters->white_time && !parameters->black_time)) {
        return false;
    }

    // Stop if time is up
    if (parameters->move_time && elapsed_time >= parameters->move_time) {
        return true;
    }

    // Calculate target time to search
    double factor = game_ply < 20 ? 1 : 2 - game_ply / 20.0;
    double time =
        player == WHITE
            ? parameters->white_increment + parameters->white_time / 100.0
            : parameters->black_increment + parameters->black_time / 100.0;
    double target = time * factor;

    // Calculate expected time to search based on previous search
    double expected_time = 5.75 * elapsed_time;

    return expected_time >= target;
}
