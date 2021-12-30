#include "move.h"
#include "move_generation.h"

static const int castling_mask[64] = {
    13, 15, 15, 15, 12, 15, 15, 14, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 7,  15, 15, 15, 3,  15, 15, 11,
};

static inline void move_piece(Board *board, int start, int end);
static inline void move_capture(Board *board, int start, int end);
static inline void move_castle(Board *board, int start, int end);
static inline void move_enpassant(Board *board, int start, int end);
static inline void move_promotion(Board *board, int square, int piece);

Move encode_move(int start, int end, int flag, int promotion) {
    return start | (end << 6) | (flag << 12) | (promotion << 14);
}

void move(Board *board, Move move) {
    State state = board->state[board->ply];

    int start = get_move_start(move);
    int end = get_move_end(move);
    int flag = get_move_flag(move);
    int piece = board->board[start];
    int capture = board->board[end];

    state.capture = capture;
    state.castling &= castling_mask[start] & castling_mask[end];
    state.enpassant = NO_SQUARE;
    state.draw_ply++;

    if (flag == CASTLING_FLAG) {
        move_castle(board, start, end);
    } else if (flag == ENPASSANT_FLAG) {
        move_enpassant(board, start, end);
        state.draw_ply = 0;
    } else {
        if (capture == NO_PIECE) {
            move_piece(board, start, end);
        } else {
            move_capture(board, start, end);
            state.draw_ply = 0;
        }

        if (get_piece(piece) == PAWN) {
            /* Double pawn push */
            if ((start ^ end) == 16) {
                state.enpassant = start + (get_color(piece) == WHITE ? 8 : -8);
            } else if (flag == PROMOTION_FLAG) {
                move_promotion(board, end, get_move_promotion(move) + KNIGHT);
            }

            state.draw_ply = 0;
        }
    }

    board->state[++board->ply] = state;
    board->player = !board->player;
}

static inline void move_piece(Board *board, int start, int end) {
    int piece = board->board[start];
    Bitboard pieces = create_bit(start) | create_bit(end);

    board->pieces[piece] ^= pieces;
    board->occupancies[get_color(piece)] ^= pieces;
    board->occupancies[2] ^= pieces;

    board->board[start] = NO_PIECE;
    board->board[end] = piece;
}

static inline void move_capture(Board *board, int start, int end) {
    int piece = board->board[start];
    int capture = board->board[end];
    Bitboard start_bitboard = create_bit(start), end_bitboard = create_bit(end);

    board->pieces[piece] ^= start_bitboard | end_bitboard;
    board->occupancies[get_color(piece)] ^= start_bitboard | end_bitboard;
    board->pieces[capture] ^= end_bitboard;
    board->occupancies[get_color(capture)] ^= end_bitboard;
    board->occupancies[2] ^= start_bitboard;

    board->board[start] = NO_PIECE;
    board->board[end] = piece;
}

static inline void move_castle(Board *board, int start, int end) {
    int king = board->board[start];
    int rook = board->board[end];
    int side = start < end;
    Bitboard kings = create_bit(start) | create_bit(start + (side ? 2 : -2));
    Bitboard rooks = create_bit(end) | create_bit(start + (side ? 1 : -1));

    board->pieces[king] ^= kings;
    board->pieces[rook] ^= rooks;
    board->occupancies[get_color(start)] ^= kings | rooks;
    board->occupancies[2] ^= kings | rooks;

    board->board[start] = NO_PIECE;
    board->board[end] = NO_PIECE;
    board->board[start + (side ? 2 : -2)] = king;
    board->board[start + (side ? 1 : -1)] = rook;
}

static inline void move_enpassant(Board *board, int start, int end) {
    int pawn = board->board[start];
    int enemy = 8 * (start / 8) + (end & 7);
    Bitboard pawns = create_bit(start) | create_bit(end);
    Bitboard enemies = create_bit(enemy);

    board->pieces[pawn] ^= pawns;
    board->pieces[board->board[enemy]] ^= enemies;
    board->occupancies[get_color(pawn)] ^= pawns;
    board->occupancies[!get_color(pawn)] ^= enemies;
    board->occupancies[2] ^= pawns | enemies;

    board->board[start] = NO_PIECE;
    board->board[end] = pawn;
    board->board[enemy] = NO_PIECE;
}

static inline void move_promotion(Board *board, int square, int piece) {
    int pawn = board->board[square];
    Bitboard bitboard = create_bit(square);

    if (get_color(piece) == BLACK) {
        piece += 8;
    }

    board->pieces[pawn] ^= bitboard;
    board->pieces[piece] ^= bitboard;

    board->board[square] = piece;
}
