#include "move_generation.h"
#include "attacks.h"
#include "move.h"

static inline void generate_piece_moves(const Board *board, Move *moves,
                                        int *count, int piece, bool capture);
static inline void generate_pawn_moves(const Board *board, Move *moves,
                                       int *count);
static inline void generate_pawn_captures(const Board *board, Move *moves,
                                          int *count);

// Generate pseudo legal moves
int generate_moves(const Board *board, Move *moves) {
    int count = 0;

    // Generate moves for each piece type
    generate_pawn_moves(board, moves, &count);
    generate_piece_moves(board, moves, &count, KNIGHT, false);
    generate_piece_moves(board, moves, &count, BISHOP, false);
    generate_piece_moves(board, moves, &count, ROOK, false);
    generate_piece_moves(board, moves, &count, QUEEN, false);
    generate_piece_moves(board, moves, &count, KING, false);

    // Generate castling moves if they are legal
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

// Generate capture moves and queen promotions
int generate_quiescence_moves(const Board *board, Move *moves) {
    int count = 0;

    generate_pawn_captures(board, moves, &count);
    generate_piece_moves(board, moves, &count, KNIGHT, true);
    generate_piece_moves(board, moves, &count, BISHOP, true);
    generate_piece_moves(board, moves, &count, ROOK, true);
    generate_piece_moves(board, moves, &count, QUEEN, true);
    generate_piece_moves(board, moves, &count, KING, true);

    return count;
}

// Generate only legal moves
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

// Generate all moves for a piece type
static inline void generate_piece_moves(const Board *board, Move *moves,
                                        int *count, int piece, bool capture) {
    Bitboard pieces = board->pieces[make_piece(piece, board->player)];

    // Iterate over each square in the piece bitboard
    while (pieces) {
        int start = pop_lsb(&pieces);

        // Get only captures or all moves
        Bitboard attacks = capture ? get_attacks(board, start, piece) &
                                         board->occupancies[!board->player]
                                   : get_attacks(board, start, piece);
        while (attacks) {
            moves[(*count)++] = encode_move(start, pop_lsb(&attacks), 0, 0);
        }
    }
}

// Generate all pawn moves
static inline void generate_pawn_moves(const Board *board, Move *moves,
                                       int *count) {
    int piece = make_piece(PAWN, board->player);
    int up = UP, upleft = UPLEFT, upright = UPRIGHT;
    Bitboard rank3 = UINT64_C(0xFF0000), rank7 = UINT64_C(0xFF000000000000);

    // Flip direction if player is black
    if (board->player == BLACK) {
        rank3 = UINT64_C(0xFF0000000000);
        rank7 = UINT64_C(0xFF00);
        up = DOWN;
        upleft = DOWNLEFT;
        upright = DOWNRIGHT;
    }

    Bitboard pawns = board->pieces[piece] & ~rank7;
    Bitboard empty = ~board->occupancies[2];

    Bitboard seventh_pawns = board->pieces[piece] & rank7;
    Bitboard single_push = shift_bit(pawns, up) & empty;
    Bitboard double_push = shift_bit(single_push & rank3, up) & empty;

    // Promotion moves
    if (seventh_pawns) {
        Bitboard enemies = board->occupancies[!board->player];
        Bitboard left = shift_bit(seventh_pawns, upleft) & enemies;
        Bitboard right = shift_bit(seventh_pawns, upright) & enemies;
        Bitboard middle = shift_bit(seventh_pawns, up) & empty;

        while (left) {
            int end = pop_lsb(&left);
            for (int i = QUEEN; i >= KNIGHT; i--) {
                moves[(*count)++] =
                    encode_move(end - upleft, end, PROMOTION, i);
            }
        }
        while (right) {
            int end = pop_lsb(&right);
            for (int i = QUEEN; i >= KNIGHT; i--) {
                moves[(*count)++] =
                    encode_move(end - upright, end, PROMOTION, i);
            }
        }
        while (middle) {
            int end = pop_lsb(&middle);
            for (int i = QUEEN; i >= KNIGHT; i--) {
                moves[(*count)++] = encode_move(end - up, end, PROMOTION, i);
            }
        }
    }

    // Pawn attacks
    while (pawns) {
        int start = pop_lsb(&pawns);

        Bitboard attacks = get_attacks(board, start, PAWN);
        while (attacks) {
            moves[(*count)++] = encode_move(start, pop_lsb(&attacks), 0, 0);
        }
    }

    // Single and double push pawn moves
    while (single_push) {
        int end = pop_lsb(&single_push);
        moves[(*count)++] = encode_move(end - up, end, 0, 0);
    }
    while (double_push) {
        int end = pop_lsb(&double_push);
        moves[(*count)++] = encode_move(end - up - up, end, 0, 0);
    }

    // Enpassant moves
    int ep = board->state[board->ply].enpassant;
    if (ep != NO_SQUARE) {
        if ((ep & 7) != 7 && board->board[ep - upleft] == piece) {
            moves[(*count)++] = encode_move(ep - upleft, ep, ENPASSANT, 0);
        }
        if ((ep & 7) != 0 && board->board[ep - upright] == piece) {
            moves[(*count)++] = encode_move(ep - upright, ep, ENPASSANT, 0);
        }
    }
}

// Generate all pawn captures and queen promotions
static inline void generate_pawn_captures(const Board *board, Move *moves,
                                          int *count) {
    int piece = make_piece(PAWN, board->player);
    int up = UP, upleft = UPLEFT, upright = UPRIGHT;
    Bitboard rank7 = UINT64_C(0xFF000000000000);

    // Flip direction if player is black
    if (board->player == BLACK) {
        rank7 = UINT64_C(0xFF00);
        up = DOWN;
        upleft = DOWNLEFT;
        upright = DOWNRIGHT;
    }

    Bitboard pawns = board->pieces[piece] & ~rank7;
    Bitboard empty = ~board->occupancies[2];
    Bitboard seventh_pawns = board->pieces[piece] & rank7;

    // Queen promotion moves
    if (seventh_pawns) {
        Bitboard enemies = board->occupancies[!board->player];
        Bitboard left = shift_bit(seventh_pawns, upleft) & enemies;
        Bitboard right = shift_bit(seventh_pawns, upright) & enemies;
        Bitboard middle = shift_bit(seventh_pawns, up) & empty;

        while (left) {
            int end = pop_lsb(&left);
            moves[(*count)++] =
                encode_move(end - upleft, end, PROMOTION, QUEEN);
        }
        while (right) {
            int end = pop_lsb(&right);
            moves[(*count)++] =
                encode_move(end - upright, end, PROMOTION, QUEEN);
        }
        while (middle) {
            int end = pop_lsb(&middle);
            moves[(*count)++] = encode_move(end - up, end, PROMOTION, QUEEN);
        }
    }

    // Pawn attacks
    while (pawns) {
        int start = pop_lsb(&pawns);

        Bitboard attacks = get_attacks(board, start, PAWN);
        while (attacks) {
            moves[(*count)++] = encode_move(start, pop_lsb(&attacks), 0, 0);
        }
    }
}
