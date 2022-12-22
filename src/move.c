#include "move.h"
#include "move_generation.h"
#include "search.h"
#include "transposition.h"

// Mask for castling rights lost if piece on square changes
static const int castling_mask[64] = {
    13, 15, 15, 15, 12, 15, 15, 14, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 7,  15, 15, 15, 3,  15, 15, 11,
};

// Keys for position hashing
extern U64 piece_key[16][64];
extern U64 castling_key[16];
extern U64 enpassant_key[64 + 1];
extern U64 side_key;

static inline void move_piece(Board *board, int start, int end);
static inline void move_capture(Board *board, int start, int end);
static inline void move_castle(Board *board, int start, int end);
static inline void move_enpassant(Board *board, int start, int end, int enemy);
static inline void move_promotion(Board *board, int square, int piece);
static inline void unmove_castle(Board *board, int start, int end);
static inline void place_piece(Board *board, int square, int piece);

// Print move with start square, end square, and special flags
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

// Make a move on the board
void make_move(Board *board, Move move) {
    State state = board->state[board->ply];

    int start = get_move_start(move);
    int end = get_move_end(move);
    int flag = get_move_flag(move);
    int piece = board->board[start];
    int capture = board->board[end];

    // Save current board hash for repetition detection
    board->repetitions[board->ply] = board->hash;

    // Set current state
    state.capture = capture;
    state.castling &= castling_mask[start] & castling_mask[end];
    state.enpassant = NO_SQUARE;
    state.draw_ply++;

    if (flag == CASTLING) {
        // Castle move
        move_castle(board, start, end);
    } else if (flag == ENPASSANT) {
        // Enpassant move
        int enemy = 8 * (start / 8) + (end & 7);
        state.capture = board->board[enemy];
        move_enpassant(board, start, end, enemy);
        state.draw_ply = 0;
    } else {
        if (capture == NO_PIECE) {
            // Quiet move
            move_piece(board, start, end);
        } else {
            // Capture move
            move_capture(board, start, end);
            state.draw_ply = 0;
        }

        if (get_piece_type(piece) == PAWN) {
            // Double pawn push sets enpassant square
            if ((start ^ end) == 16) {
                state.enpassant =
                    start + (get_piece_color(piece) == WHITE ? 8 : -8);
            } else if (flag == PROMOTION) {
                // Promotion move
                move_promotion(board, end, get_move_promotion(move) + KNIGHT);
            }

            state.draw_ply = 0;
        }
    }

    // Update position hashing for castling and enpassant
    board->hash ^= castling_key[board->state[board->ply].castling] ^
                   castling_key[state.castling] ^
                   enpassant_key[board->state[board->ply].enpassant] ^
                   enpassant_key[state.enpassant] ^ side_key;

    // Increment ply
    board->ply++;

    // Save state and switch player
    board->state[board->ply] = state;
    board->player = !board->player;
}

// Undo a move on the board
void unmake_move(Board *board, Move move) {
    State state = board->state[board->ply];
    int start = get_move_start(move);
    int end = get_move_end(move);
    int flag = get_move_flag(move);

    // Switch player
    board->player = !board->player;

    if (flag == CASTLING) {
        // Undo castle
        unmove_castle(board, start, end);
    } else {
        // Move end square to start square
        move_piece(board, end, start);

        if (state.capture != NO_PIECE) {
            // Place captured piece back on the board
            if (flag == ENPASSANT) {
                place_piece(board, 8 * (start / 8) + (end & 7), state.capture);
            } else {
                place_piece(board, end, state.capture);
            }
        }

        // Replace promoted piece with pawn
        if (flag == PROMOTION) {
            move_promotion(board, start, PAWN);
        }
    }

    // Decrement ply
    board->ply--;

    // Update position hashing for castling and enpassant
    board->hash ^= castling_key[board->state[board->ply].castling] ^
                   castling_key[state.castling] ^
                   enpassant_key[board->state[board->ply].enpassant] ^
                   enpassant_key[state.enpassant] ^ side_key;
}

// Move piece and update bitboards
static inline void move_piece(Board *board, int start, int end) {
    int piece = board->board[start];
    Bitboard pieces = create_bit(start) | create_bit(end);

    board->pieces[piece] ^= pieces;
    board->occupancies[get_piece_color(piece)] ^= pieces;
    board->occupancies[2] ^= pieces;

    board->board[start] = NO_PIECE;
    board->board[end] = piece;

    board->hash ^= piece_key[piece][start] ^ piece_key[piece][end];
}

// Move piece, capture, and update bitboards
static inline void move_capture(Board *board, int start, int end) {
    int piece = board->board[start];
    int capture = board->board[end];
    Bitboard start_bitboard = create_bit(start), end_bitboard = create_bit(end);

    board->pieces[piece] ^= start_bitboard | end_bitboard;
    board->occupancies[get_piece_color(piece)] ^= start_bitboard | end_bitboard;
    board->pieces[capture] ^= end_bitboard;
    board->occupancies[get_piece_color(capture)] ^= end_bitboard;
    board->occupancies[2] ^= start_bitboard;

    board->board[start] = NO_PIECE;
    board->board[end] = piece;

    board->hash ^= piece_key[piece][start] ^ piece_key[piece][end] ^
                   piece_key[capture][end];
}

// Castle and update bitboards
static inline void move_castle(Board *board, int start, int end) {
    int side = start < end;
    int king_square = start + (side ? 2 : -2);
    int rook_square = start + (side ? 1 : -1);
    int king = board->board[start];
    int rook = king - 2;
    Bitboard kings = create_bit(start) | create_bit(king_square);
    Bitboard rooks = create_bit(end) | create_bit(rook_square);

    board->pieces[king] ^= kings;
    board->pieces[rook] ^= rooks;
    board->occupancies[get_piece_color(king)] ^= kings | rooks;
    board->occupancies[2] ^= kings | rooks;

    board->board[start] = NO_PIECE;
    board->board[end] = NO_PIECE;
    board->board[king_square] = king;
    board->board[rook_square] = rook;

    board->hash ^= piece_key[king][king_square] ^ piece_key[king][start] ^
                   piece_key[rook][rook_square] ^ piece_key[rook][end];
}

// Capture enpassant and update bitboards
static inline void move_enpassant(Board *board, int start, int end, int enemy) {
    int pawn = board->board[start];
    int enemy_piece = board->board[enemy];
    Bitboard pawns = create_bit(start) | create_bit(end);
    Bitboard enemies = create_bit(enemy);

    board->pieces[pawn] ^= pawns;
    board->pieces[enemy_piece] ^= enemies;
    board->occupancies[get_piece_color(pawn)] ^= pawns;
    board->occupancies[!get_piece_color(pawn)] ^= enemies;
    board->occupancies[2] ^= pawns | enemies;

    board->board[start] = NO_PIECE;
    board->board[end] = pawn;
    board->board[enemy] = NO_PIECE;

    board->hash ^= piece_key[pawn][start] ^ piece_key[pawn][end] ^
                   piece_key[enemy_piece][enemy];
}

// Place promoted piece on square and update bitboards
static inline void move_promotion(Board *board, int square, int piece) {
    int pawn = board->board[square];
    Bitboard bitboard = create_bit(square);

    if (get_piece_color(pawn) == BLACK) {
        piece += 8;
    }

    board->pieces[pawn] ^= bitboard;
    board->pieces[piece] ^= bitboard;

    board->board[square] = piece;

    board->hash ^= piece_key[pawn][square] ^ piece_key[piece][square];
}

// Undo castle and update bitboards
static inline void unmove_castle(Board *board, int start, int end) {
    int side = start < end;
    int king_square = start + (side ? 2 : -2);
    int rook_square = start + (side ? 1 : -1);
    int king = board->board[king_square];
    int rook = king - 2;
    Bitboard kings = create_bit(start) | create_bit(king_square);
    Bitboard rooks = create_bit(end) | create_bit(rook_square);

    board->pieces[king] ^= kings;
    board->pieces[rook] ^= rooks;
    board->occupancies[get_piece_color(king)] ^= kings | rooks;
    board->occupancies[2] ^= kings | rooks;

    board->board[start] = king;
    board->board[end] = rook;
    board->board[king_square] = NO_PIECE;
    board->board[rook_square] = NO_PIECE;

    board->hash ^= piece_key[king][king_square] ^ piece_key[king][start] ^
                   piece_key[rook][rook_square] ^ piece_key[rook][end];
}

// Place piece and update bitboards
static inline void place_piece(Board *board, int square, int piece) {
    Bitboard bitboard = create_bit(square);

    board->pieces[piece] ^= bitboard;
    board->occupancies[get_piece_color(piece)] ^= bitboard;
    board->occupancies[2] ^= bitboard;

    board->board[square] = piece;

    board->hash ^= piece_key[piece][square];
}
