#include "move.h"
#include "board.h"
#include "move_generation.h"

static inline void update_piece(Fast start, Fast end);

/* Changes board based on move (does not check for legality) */
void move_piece(const Move *move) {
    /* Resets ply if pawn move or capture for 50 move rule */
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
        Fast square = 16 * get_rank(move->start) + get_file(move->end);
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
    game_moves[search_pos++] = *move;
}

/* Reverses move_piece function */
void unmove_piece() {
    Move *move = &game_moves[--search_pos];

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
    case ENPASSANT:
        int square = 16 * get_rank(move->start) + get_file(move->end);
        board.colors[square] = board.player;
        board.pieces[square] = PAWN;
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
int move_legal(Fast start, Fast end, Fast promotion) {
    /* Start and end must be in the board and be different colors */
    /* Starting square must be the player to move's piece */
    if (invalid_square(start) || invalid_square(end) ||
        board.colors[start] != board.player ||
        board.colors[end] == board.player) {
        return FAILURE;
    }

    /* Generate pseudo legal moves */
    Move moves[MAX_MOVES];
    int count = generate_moves(moves);

    /* NOTE: Could change moves from array to hashset later, but this code is
    for checking if player moves are legal, so performance is not critical */

    /* Iterates through all legal moves and checks if the move is in there */
    for (int i = 0; i < count; i++) {
        if (moves[i].start == start && moves[i].end == end &&
            (!promotion || moves[i].flag == promotion)) {
            move_piece(&moves[i]);
            if (in_check()) {
                unmove_piece();
                return FAILURE;
            }
            return SUCCESS;
        }
    }

    return FAILURE;
}

/* Moves piece from start to end and deletes start piece */
static inline void update_piece(Fast start, Fast end) {
    board.colors[end] = board.colors[start];
    board.pieces[end] = board.pieces[start];
    board.colors[start] = EMPTY_COLOR;
    board.pieces[start] = EMPTY_PIECE;
}