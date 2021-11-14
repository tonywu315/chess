#include "move.h"
#include "move_generation.h"
#include "search.h"

static inline void update_piece(U8 start, U8 end);

/* Changes board based on move (does not check for legality) */
void move_piece(const Move *move) {
    /* Checks that move is valid is debug flag is set */
    debug_assert(move->start != move->end);
    debug_assert(board.colors[move->start] == board.player);
    debug_assert(board.colors[move->end] != board.player);
    debug_assert(board.pieces[move->start] != EMPTY_PIECE);
    debug_assert(!invalid_square(move->start));
    debug_assert(!invalid_square(move->end));

    /* Resets ply if pawn move or capture for 50 move rule. Enpassant and
    promotion captures do not have capture flag, but are covered by pawn move */
    board.ply++;
    if (board.pieces[move->start] == PAWN || move->flag == CAPTURE) {
        board.ply = 0;
    }

    /* Updates king position */
    if (board.pieces[move->start] == KING) {
        board.king[board.player - 1] = move->end;
    }

    /* Updates board */
    update_piece(move->start, move->end);

    /* Moving king or rook removes castle option for that side */
    switch (move->start) {
    case E1:
        board.castle &= 15 - CASTLE_WK - CASTLE_WQ;
        break;
    case E8:
        board.castle &= 15 - CASTLE_BK - CASTLE_BQ;
        break;
    case H1:
        board.castle &= 15 - CASTLE_WK;
        break;
    case A1:
        board.castle &= 15 - CASTLE_WQ;
        break;
    case H8:
        board.castle &= 15 - CASTLE_BK;
        break;
    case A8:
        board.castle &= 15 - CASTLE_BQ;
        break;
    }

    /* Additional moves based on flag */
    switch (move->flag) {
    case DOUBLE:
        board.enpassant = (move->start + move->end) / 2;
        break;
    case ENPASSANT:; /* Semicolon is necessary to compile */
        U8 square = 16 * get_rank(move->start) + get_file(move->end);
        board.colors[square] = EMPTY_COLOR;
        board.pieces[square] = EMPTY_PIECE;
        break;
    case CASTLE_WK:
        update_piece(H1, F1);
        board.castle &= 15 - CASTLE_WK - CASTLE_WQ;
        break;
    case CASTLE_WQ:
        update_piece(A1, D1);
        board.castle &= 15 - CASTLE_WK - CASTLE_WQ;
        break;
    case CASTLE_BK:
        update_piece(H8, F8);
        board.castle &= 15 - CASTLE_BK - CASTLE_BQ;
        break;
    case CASTLE_BQ:
        update_piece(A8, D8);
        board.castle &= 15 - CASTLE_BK - CASTLE_BQ;
        break;
    case PROMOTION_N:
        board.pieces[move->end] = KNIGHT;
        break;
    case PROMOTION_B:
        board.pieces[move->end] = BISHOP;
        break;
    case PROMOTION_R:
        board.pieces[move->end] = ROOK;
        break;
    case PROMOTION_Q:
        board.pieces[move->end] = QUEEN;
        break;
    }

    /* Removes enpassant flag after 1 move */
    if (move->flag != DOUBLE) {
        board.enpassant = -1;
    }

    /* Switches players, increments search, and adds to moves */
    board.player = 3 - board.player;
    game_moves[game_position++] = *move;
}

/* Reverses move_piece function */
void unmove_piece() {
    Move *move = &game_moves[--game_position];

    /* Resets castle, enpassant, and ply */
    board.castle = move->castle;
    board.enpassant = move->enpassant;
    board.ply = move->ply;

    /* Resets king position */
    if (board.pieces[move->end] == KING) {
        board.king[2 - board.player] = move->start;
    }

    /* Undoes move and replaces any captured piece */
    update_piece(move->end, move->start);
    if (move->captured) {
        board.colors[move->end] = board.player;
        board.pieces[move->end] = move->captured;
    }

    /* Additional moves based on flag */
    switch (move->flag) {
    case ENPASSANT:; /* Semicolon is necessary to compile */
        int square = 16 * get_rank(move->start) + get_file(move->end);
        board.colors[square] = board.player;
        board.pieces[square] = PAWN;
        break;
    case CASTLE_WK:
        update_piece(F1, H1);
        break;
    case CASTLE_WQ:
        update_piece(D1, A1);
        break;
    case CASTLE_BK:
        update_piece(F8, H8);
        break;
    case CASTLE_BQ:
        update_piece(D8, A8);
        break;
    case PROMOTION_N:
    case PROMOTION_B:
    case PROMOTION_R:
    case PROMOTION_Q:
        board.pieces[move->start] = PAWN;
        break;
    }

    /* Switches players and decrements search */
    board.player = 3 - board.player;
}

/* Moves piece from start to end if it is legal */
int move_legal(const Move *move) {
    /* Start and end must be in the board and be different colors */
    /* Starting square must be the player to move's piece */
    if (invalid_square(move->start) || invalid_square(move->end) ||
        board.colors[move->start] != board.player ||
        board.colors[move->end] == board.player) {
        return FAILURE;
    }

    /* Generate legal moves */
    Move moves[MAX_MOVES];
    int count = generate_legal_moves(moves);

    /* NOTE: Could change moves from array to hashset later, but this code only
    runs once per turn, so performance is not critical */

    /* Iterates through all legal moves and checks if the move is in there */
    for (int i = 0; i < count; i++) {
        if (moves[i].start == move->start && moves[i].end == move->end &&
            (move->flag < PROMOTION_N || moves[i].flag == move->flag)) {
            move_piece(&moves[i]);
            return SUCCESS;
        }
    }

    return FAILURE;
}

/* Moves piece for computer and returns score */
int move_computer(int depth) {
    Line mainline;
    int score = alpha_beta(-INT_MAX, INT_MAX, 0, depth, &mainline);

    /* Checks if engine is going to be checkmated */
    if (score == -INT_MAX) {
        /* TODO: delay checkmate as long as possible */
        Move moves[MAX_MOVES];
        generate_legal_moves(moves);
        move_piece(&moves[0]);

        debug_print("%s\n", "Computer is getting checkmated");
    } else {
        move_piece(&mainline.moves[0]);
    }

    return score;
}

/* Moves piece from start to end and deletes start piece */
static inline void update_piece(U8 start, U8 end) {
    board.colors[end] = board.colors[start];
    board.pieces[end] = board.pieces[start];
    board.colors[start] = EMPTY_COLOR;
    board.pieces[start] = EMPTY_PIECE;
}
