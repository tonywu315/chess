#include "move_order.h"

static inline int mvv_lva(int attacker, int victim);

// Score moves and save in move list
void score_moves(Board *board, Move *moves, MoveList *move_list, int length,
                 Move tt_move) {
    for (int i = 0; i < length; i++) {
        Move move = moves[i];

        if (move == tt_move) {
            move_list[i].move = move;
            move_list[i].score = TT_MOVE;
        } else {
            int flag = get_move_flag(move);

            if (flag == PROMOTION) {
                move_list[i].move = move;
                move_list[i].score = PxR + get_move_promotion(move);
            } else if (flag == ENPASSANT) {
                move_list[i].move = move;
                move_list[i].score = PxP;
            } else if (flag == CASTLING) {
                move_list[i].move = move;
                move_list[i].score = QUIET_MOVE + 1;
            } else {
                int capture = board->board[get_move_end(move)];
                if (capture != NO_PIECE) {
                    move_list[i].move = moves[i];
                    move_list[i].score =
                        mvv_lva(board->board[get_move_start(move)], capture);
                } else {
                    move_list[i].move = moves[i];
                    move_list[i].score = QUIET_MOVE;
                }
            }
        }
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
