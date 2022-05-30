#include "move_order.h"

// Score each move
void score_moves(Board *board, Move *moves, MoveList *move_list, int length,
                 Move tt_move) {
    // clang-format off
    const int mvv_lva[][15] = {
        [W_QUEEN] = {
            [W_PAWN] = 15,   [B_PAWN] = 15,   [W_KNIGHT] = 14, [B_KNIGHT] = 14,
            [W_BISHOP] = 13, [B_BISHOP] = 13, [W_ROOK] = 12,   [B_ROOK] = 12,
            [W_QUEEN] = 1,   [B_QUEEN] = 1,   [W_KING] = 11,   [B_KING] = 11,
        },
        [B_QUEEN] = {
            [W_PAWN] = 15,   [B_PAWN] = 15,   [W_KNIGHT] = 14, [B_KNIGHT] = 14,
            [W_BISHOP] = 13, [B_BISHOP] = 13, [W_ROOK] = 12,   [B_ROOK] = 12,
            [W_QUEEN] = 1,   [B_QUEEN] = 1,   [W_KING] = 11,   [B_KING] = 11,
        },
        [W_ROOK] = {
            [W_PAWN] = 9,    [B_PAWN] = 9,    [W_KNIGHT] = 8, [B_KNIGHT] = 8,
            [W_BISHOP] = 7,  [B_BISHOP] = 7,  [W_ROOK] = 1,   [B_ROOK] = 1,
            [W_QUEEN] = -13, [B_QUEEN] = -13, [W_KING] = 6,   [B_KING] = 6,
        },
        [B_ROOK] = {
            [W_PAWN] = 9,    [B_PAWN] = 9,    [W_KNIGHT] = 8, [B_KNIGHT] = 8,
            [W_BISHOP] = 7,  [B_BISHOP] = 7,  [W_ROOK] = 1,   [B_ROOK] = 1,
            [W_QUEEN] = -13, [B_QUEEN] = -13, [W_KING] = 6,   [B_KING] = 6,
        },
        [W_BISHOP] = {
            [W_PAWN] = 5,    [B_PAWN] = 5,    [W_KNIGHT] = 2, [B_KNIGHT] = 2,
            [W_BISHOP] = 1,  [B_BISHOP] = 1,  [W_ROOK] = -12, [B_ROOK] = -12,
            [W_QUEEN] = -16, [B_QUEEN] = -16, [W_KING] = 4,   [B_KING] = 4,
        },
        [B_BISHOP] = {
            [W_PAWN] = 5,    [B_PAWN] = 5,    [W_KNIGHT] = 2, [B_KNIGHT] = 2,
            [W_BISHOP] = 1,  [B_BISHOP] = 1,  [W_ROOK] = -12, [B_ROOK] = -12,
            [W_QUEEN] = -16, [B_QUEEN] = -16, [W_KING] = 4,   [B_KING] = 4,
        },
        [W_KNIGHT] = {
            [W_PAWN] = 5,    [B_PAWN] = 5,    [W_KNIGHT] = 1, [B_KNIGHT] = 1,
            [W_BISHOP] = 0,  [B_BISHOP] = 0,  [W_ROOK] = -12, [B_ROOK] = -12,
            [W_QUEEN] = -16, [B_QUEEN] = -16, [W_KING] = 4,   [B_KING] = 4,
        },
        [B_KNIGHT] = {
            [W_PAWN] = 5,    [B_PAWN] = 5,    [W_KNIGHT] = 1, [B_KNIGHT] = 1,
            [W_BISHOP] = 0,  [B_BISHOP] = 0,  [W_ROOK] = -12, [B_ROOK] = -12,
            [W_QUEEN] = -16, [B_QUEEN] = -16, [W_KING] = 4,   [B_KING] = 4,
        },
        [W_PAWN] = {
            [W_PAWN] = 0,     [B_PAWN] = 0,     [W_KNIGHT] = -11, [B_KNIGHT] = -11,
            [W_BISHOP] = -11, [B_BISHOP] = -11, [W_ROOK] = -14,   [B_ROOK] = -14,
            [W_QUEEN] = -18,  [B_QUEEN] = -18,  [W_KING] = 3,     [B_KING] = 3,
        },
        [B_PAWN] = {
            [W_PAWN] = 0,     [B_PAWN] = 0,     [W_KNIGHT] = -11, [B_KNIGHT] = -11,
            [W_BISHOP] = -11, [B_BISHOP] = -11, [W_ROOK] = -14,   [B_ROOK] = -14,
            [W_QUEEN] = -18,  [B_QUEEN] = -18,  [W_KING] = 3,     [B_KING] = 3,
        },
    };
    // clang-format on

    for (int i = 0; i < length; i++) {
        Move move = moves[i];
        if (move == tt_move) {
            move_list[i].move = move;
            move_list[i].score = TT_MOVE;
        } else {
            int start = get_move_start(move);
            int end = get_move_end(move);
            int capture = board->board[end];
            int flag = get_move_flag(move);

            if (flag == PROMOTION) {
                move_list[i].move = move;
                move_list[i].score = 50;
            } else if (flag == ENPASSANT) {
                move_list[i].move = move;
                move_list[i].score = 0;
            } else if (flag == CASTLING) {
                move_list[i].move = move;
                move_list[i].score = -50;
            } else if (capture != NO_PIECE) {
                move_list[i].move = moves[i];
                move_list[i].score = mvv_lva[capture][board->board[start]];
            } else {
                move_list[i].move = moves[i];
                move_list[i].score = QUIET_MOVE;
            }  
        }
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
