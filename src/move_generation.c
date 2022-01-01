#include "move_generation.h"
#include "attacks.h"
#include "bitboard.h"

#include "move.h"

static void generate_pawn_moves(const Board *board, Move *moves, int *count);
static void generate_piece_moves(const Board *board, Move *moves, int *count,
                                 int piece);

void print_move(Move move) {
    static const char *promotion[4] = {"knight", "bishop", "rook", "queen"};
    printf("%s%s", get_coordinates(get_move_start(move)),
           get_coordinates(get_move_end(move)));
    switch (get_move_flag(move)) {
    case PROMOTION:
        printf(" promotion %s\n", promotion[get_move_promotion(move)]);
        break;
    case ENPASSANT:
        printf(" enpassant\n");
        break;
    case CASTLING:
        printf(" castle\n");
        break;
    default:
        printf("\n");
        break;
    }
}

// bool is_legal(Board *board, Move move) {
//     int start = get_move_start(move), end = get_move_end(move);
//     int flag = get_move_flag(move);

//     if (flag == CASTLING) {
//         int direction = start < end ? 1 : -1;
//         for (int square = start; square != end; square += direction) {
//             if (is_attacked(board, square, !board->player)) {
//                 return false;
//             }
//         }

//         return true;
//     }

//     if (get_piece(board->board[start]) == KING) {
//         return !is_attacked(board, end, !board->player);
//     }

//     bool legal = true;
//     int king_square = get_lsb(board->pieces[make_piece(KING,
//     board->player)]);

//     make_move(board, move);
//     if (is_attacked(board, king_square, board->player)) {
//         legal = false;
//     }
//     unmake_move(board, move);

//     return true;
// }

int generate_moves(const Board *board, Move *moves) {
    int player = board->player, count = 0;

    generate_pawn_moves(board, moves, &count);
    generate_piece_moves(board, moves, &count, make_piece(KNIGHT, player));
    generate_piece_moves(board, moves, &count, make_piece(BISHOP, player));
    generate_piece_moves(board, moves, &count, make_piece(ROOK, player));
    generate_piece_moves(board, moves, &count, make_piece(QUEEN, player));
    generate_piece_moves(board, moves, &count, make_piece(KING, player));

    int castling = board->state[board->ply].castling;
    if (castling) {
        if (board->player == WHITE) {
            if ((CASTLE_WK & castling) &&
                !(board->occupancies[2] & UINT64_C(0x60))) {
                if (!is_attacked(board, E1, !board->player) &&
                    !is_attacked(board, F1, !board->player) &&
                    !is_attacked(board, G1, !board->player)) {
                    moves[count++] = UINT16_C(0xF1C4);
                }
            }
            if ((CASTLE_WQ & castling) &&
                !(board->occupancies[2] & UINT64_C(0xE))) {
                if (!is_attacked(board, E1, !board->player) &&
                    !is_attacked(board, D1, !board->player) &&
                    !is_attacked(board, C1, !board->player)) {
                    moves[count++] = UINT16_C(0xF004);
                }
            }
        } else {
            if ((CASTLE_BK & castling) &&
                !(board->occupancies[2] & UINT64_C(0x6000000000000000))) {
                if (!is_attacked(board, E8, !board->player) &&
                    !is_attacked(board, F8, !board->player) &&
                    !is_attacked(board, G8, !board->player)) {
                    moves[count++] = UINT16_C(0xFFFC);
                }
            }
            if ((CASTLE_BQ & castling) &&
                !(board->occupancies[2] & UINT64_C(0xE00000000000000))) {
                if (!is_attacked(board, E8, !board->player) &&
                    !is_attacked(board, D8, !board->player) &&
                    !is_attacked(board, C8, !board->player)) {
                    moves[count++] = UINT16_C(0xFE3C);
                }
            }
        }
    }

    return count;
}

int generate_legal_moves(Board *board, Move *moves) {
    Move pseudo_moves[MAX_MOVES];
    int count = 0, pseudo_count = generate_moves(board, pseudo_moves);

    for (int i = 0; i < pseudo_count; i++) {
        make_move(board, pseudo_moves[i]);
        if (!in_check(board, !board->player)) {
            moves[count++] = pseudo_moves[i];
        }
        unmake_move(board, pseudo_moves[i]);
    }

    return count;
}

static void generate_piece_moves(const Board *board, Move *moves, int *count,
                                 int piece) {
    Bitboard pieces = board->pieces[piece];

    while (pieces) {
        int start = pop_lsb(&pieces);

        Bitboard attacks = get_attacks(board, start, piece);
        while (attacks) {
            moves[(*count)++] = encode_move(start, pop_lsb(&attacks), 0, 0);
        }
    }
}

static void generate_pawn_moves(const Board *board, Move *moves, int *count) {
    int piece = make_piece(PAWN, board->player);
    int up = UP, upleft = UPLEFT, upright = UPRIGHT;
    Bitboard rank3 = UINT64_C(0xFF0000), rank7 = UINT64_C(0xFF000000000000);

    if (board->player == BLACK) {
        rank3 = UINT64_C(0xFF0000000000);
        rank7 = UINT64_C(0xFF00);
        up = DOWN;
        upleft = DOWNLEFT;
        upright = DOWNRIGHT;
    }

    Bitboard pawns = board->pieces[piece] & ~rank7;
    Bitboard empty = ~board->occupancies[2];

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

    while (pawns) {
        int start = pop_lsb(&pawns);

        Bitboard attacks = get_attacks(board, start, PAWN);
        while (attacks) {
            moves[(*count)++] = encode_move(start, pop_lsb(&attacks), 0, 0);
        }
    }

    int ep = board->state[board->ply].enpassant;
    if (ep != NO_SQUARE) {
        if ((ep & 7) != 7 && board->board[ep - upleft] == piece) {
            moves[(*count)++] = encode_move(ep - upleft, ep, ENPASSANT, 0);
        }
        if ((ep & 7) != 0 && board->board[ep - upright] == piece) {
            moves[(*count)++] = encode_move(ep - upright, ep, ENPASSANT, 0);
        }
    }

    Bitboard seventh = board->pieces[piece] & rank7;
    if (seventh) {
        Bitboard enemies = board->occupancies[!board->player];
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
}
