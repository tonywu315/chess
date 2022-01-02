#include "search.h"
#include "attacks.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"

static int time_over;
static void *chess_clock(void *time);

// Alpha beta algorithm
int search(Board *board, int alpha, int beta, int ply, int depth,
           Line *mainline) {
    Move moves[MAX_MOVES];
    Line line = {0, {0}};

    // Check extension
    bool check = in_check(board, board->player);
    if (check) {
        depth++;
    }

    // TODO: add quiescence search
    if (depth == 0) {
        mainline->length = 0;
        return eval(board);
    }

    // Iterate over all moves
    int count = generate_moves(board, moves), moves_count = 0;
    for (int i = 0; i < count; i++) {
        if (time_over) {
            return 0;
        }

        int score;
        make_move(board, moves[i]);

        // Removes illegal moves
        if (in_check(board, !board->player)) {
            unmake_move(board, moves[i]);
            continue;
        }

        moves_count++;

        // Calculate score of opponent
        score = -search(board, -beta, -alpha, ply + 1, depth - 1, &line);
        unmake_move(board, moves[i]);

        // Alpha cutoff
        if (score > alpha) {
            // Update mainline
            mainline->moves[0] = moves[i];
            memcpy(mainline->moves + 1, line.moves, line.length * sizeof(Move));
            mainline->length = line.length + 1;

            // Beta cutoff
            if (score >= beta) {
                alpha = beta;
                break;
            }

            alpha = score;
        }
    }

    // Checkmate and stalemate
    if (moves_count == 0) {
        alpha = check ? -INT_MAX + ply : 0;
    }

    return alpha;
}

// Search position for best move within time limit
int search_position(Board *board, Move *move, int time) {
    pthread_t tid;
    Line mainline;
    Move final_move = {0};
    int final_score = 0, score = 0;
    int alpha = -INT_MAX, beta = INT_MAX;

    time_over = false;

    pthread_create(&tid, NULL, chess_clock, (void *)&time);

    // TODO: Iterative deepening
    for (int i = 1; i <= 99; i++) {
        // TODO: implement transposition tables

        score = search(board, alpha, beta, 0, i, &mainline);

        // Stop searching if time is over
        if (!time_over) {
            final_move = mainline.moves[0];
            final_score = score;
        } else {
            if (DEBUG_FLAG) {
                printf("Depth: %d\n", i - 1);
            }

            break;
        }
    }

    pthread_join(tid, NULL);

    // Set move to mainline and return score
    *move = final_move;
    return final_score;
}

// Sleep time seconds
static void *chess_clock(void *time) {
    sleep(*(int *)time);

    time_over = true;

    return NULL;
}
