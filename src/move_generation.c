#include "move_generation.h"
#include "attacks.h"
#include "bitboard.h"

static void generate_pawn_moves(Board board, Move *moves, int *count);

int generate_moves(Board board, Move *moves) {
    int count = 0;
    Bitboard pieces = board.occupancies[board.player];

    while (pieces) {
        int start = pop_lsb(&pieces);

        Bitboard attacks = get_attacks(board, start);
        while (attacks) {
            moves[count++] = encode_move(start, pop_lsb(&attacks), 0, 0);
        }
    }

    generate_pawn_moves(board, moves, &count);

    int castling = board.state[board.ply].castling;
    if (castling) {
        if (board.player == WHITE) {
            if ((CASTLE_WK & castling) &&
                !(board.occupancies[2] & UINT64_C(0x60))) {
                moves[count++] = UINT64_C(0xF1C4);
            }
            if ((CASTLE_WQ & castling) &&
                !(board.occupancies[2] & UINT64_C(0xE))) {
                moves[count++] = UINT64_C(0xF004);
            }
        } else {
            if ((CASTLE_BK & castling) &&
                !(board.occupancies[2] & UINT64_C(0x6000000000000000))) {
                moves[count++] = UINT64_C(0xFFFC);
            }
            if ((CASTLE_BQ & castling) &&
                !(board.occupancies[2] & UINT64_C(0xE00000000000000))) {
                moves[count++] = UINT64_C(0xFE3C);
            }
        }
    }

    return count;
}

static void generate_pawn_moves(Board board, Move *moves, int *count) {
    Bitboard rank3 = UINT64_C(0xFF0000), rank7 = UINT64_C(0xFF000000000000);
    int up = UP, upleft = UPLEFT, upright = UPRIGHT, piece = W_PAWN;

    if (board.player == BLACK) {
        rank3 = UINT64_C(0xFF0000000000);
        rank7 = UINT64_C(0xFF00);
        up = DOWN;
        upleft = DOWNLEFT;
        upright = DOWNRIGHT;
        piece = B_PAWN;
    }

    Bitboard pawns = board.pieces[piece] & ~rank7;
    Bitboard enemies = board.pieces[!board.player];
    Bitboard empty = ~board.occupancies[2];

    Bitboard single_push = shift_bit(pawns, up) & empty;
    Bitboard double_push = shift_bit(single_push & rank3, up) & empty;
    while (single_push) {
        int end = pop_lsb(&single_push);
        moves[(*count)++] = encode_move(end - up, end, 0, 0);
    }
    while (double_push) {
        int end = pop_lsb(&double_push);
        moves[(*count)++] = encode_move(end - up - up, end, 0, 0);
    }

    Bitboard seventh = board.pieces[piece] & rank7;
    if (seventh) {
        Bitboard left = shift_bit(seventh, upleft) & enemies;
        Bitboard right = shift_bit(seventh, upright) & enemies;
        Bitboard middle = shift_bit(seventh, up) & empty;

        while (left) {
            int end = pop_lsb(&left);
            for (int i = KNIGHT; i <= QUEEN; i++) {
                moves[(*count)++] =
                    encode_move(end - upleft, end, PROMOTION, i);
            }
        }
        while (right) {
            int end = pop_lsb(&right);
            for (int i = KNIGHT; i <= QUEEN; i++) {
                moves[(*count)++] =
                    encode_move(end - upright, end, PROMOTION, i);
            }
        }
        while (middle) {
            int end = pop_lsb(&middle);
            for (int i = KNIGHT; i <= QUEEN; i++) {
                moves[(*count)++] = encode_move(end - up, end, PROMOTION, i);
            }
        }
    }

    int ep = board.state[board.ply].enpassant;
    if (ep != NO_SQUARE) {
        if ((ep & 7) != 7 && board.board[ep - upleft] == piece) {
            moves[(*count)++] = encode_move(ep - upleft, ep, ENPASSANT, 0);
        }
        if ((ep & 7) != 0 && board.board[ep - upright] == piece) {
            moves[(*count)++] = encode_move(ep - upright, ep, ENPASSANT, 0);
        }
    }
}
