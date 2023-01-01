#include "move_order.h"

static inline int mvv_lva(int attacker, int victim);

// Score moves and save in move list
void score_moves(Board *board, Stack *stack, Move *moves, MoveList *move_list,
                 Move tt_move, int length) {
    for (int i = 0; i < length; i++) {
        Move move = moves[i];
        int score = QUIET_MOVE;
        int flag = get_move_flag(move);
        int capture = board->board[get_move_end(move)];

        // Score move
        if (move == tt_move) {
            score = TT_MOVE;
        } else if (capture != NO_PIECE) {
            score = mvv_lva(board->board[get_move_start(move)], capture);
        } else if (move == stack->killers[0]) {
            score = KILLER_MOVE + 1;
        } else if (move == stack->killers[1]) {
            score = KILLER_MOVE;
        } else if (flag == NORMAL_MOVE) {
            score = QUIET_MOVE;
        } else if (flag == PROMOTION) {
            score = PxR + get_move_promotion(move);
        } else if (flag == ENPASSANT) {
            score = PxP;
        } else if (flag == CASTLING) {
            score = QUIET_MOVE + 1;
        }

        move_list[i].move = move;
        move_list[i].score = score;
    }
}

// Score quiescence moves and save in move list
void score_quiescence_moves(Board *board, Move *moves, MoveList *move_list,
                            int length) {
    for (int i = 0; i < length; i++) {
        Move move = moves[i];
        move_list[i].move = moves[i];
        move_list[i].score = mvv_lva(board->board[get_move_start(move)],
                                     board->board[get_move_end(move)]);
    }
}

// Swap next best move in move list to index
Move sort_moves(MoveList *move_list, int length, int index) {
    int best_index = index;

    for (int i = index + 1; i < length; i++) {
        if (move_list[i].score > move_list[best_index].score) {
            best_index = i;
        }
    }

    MoveList best_move = move_list[best_index];
    move_list[best_index] = move_list[index];
    move_list[index] = best_move;

    return best_move.move;
}

// Most Valuable Victim - Least Valuable Attacker heuristic to sort moves
static inline int mvv_lva(int attacker, int victim) {
    static const int mvv_lva[][13] = {
        {PxP, PxN, PxB, PxR, PxQ, 0, 0, 0, PxP, PxN, PxB, PxR, PxQ},
        {NxP, NxN, NxB, NxR, NxQ, 0, 0, 0, NxP, NxN, NxB, NxR, NxQ},
        {BxP, BxN, BxB, BxR, BxQ, 0, 0, 0, BxP, BxN, BxB, BxR, BxQ},
        {RxP, RxN, RxB, RxR, RxQ, 0, 0, 0, RxP, RxN, RxB, RxR, RxQ},
        {QxP, QxN, QxB, QxR, QxQ, 0, 0, 0, QxP, QxN, QxB, QxR, QxQ},
        {KxP, KxN, KxB, KxR, KxQ, 0, 0, 0, KxP, KxN, KxB, KxR, KxQ},
        {0},
        {0},
        {PxP, PxN, PxB, PxR, PxQ, 0, 0, 0, PxP, PxN, PxB, PxR, PxQ},
        {NxP, NxN, NxB, NxR, NxQ, 0, 0, 0, NxP, NxN, NxB, NxR, NxQ},
        {BxP, BxN, BxB, BxR, BxQ, 0, 0, 0, BxP, BxN, BxB, BxR, BxQ},
        {RxP, RxN, RxB, RxR, RxQ, 0, 0, 0, RxP, RxN, RxB, RxR, RxQ},
        {QxP, QxN, QxB, QxR, QxQ, 0, 0, 0, QxP, QxN, QxB, QxR, QxQ},
        {KxP, KxN, KxB, KxR, KxQ, 0, 0, 0, KxP, KxN, KxB, KxR, KxQ},
    };
    return mvv_lva[attacker][victim];
}
