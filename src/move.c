#include "move.h"
#include "board.h"
#include "constants.h"
#include <stdio.h>

static const char vectors[5][8] = {
    {33, 31, 18, 14, -14, -18, -31, -33},
    {UPRIGHT, UPLEFT, DOWNRIGHT, DOWNLEFT},
    {UP, RIGHT, LEFT, DOWN},
    {UPRIGHT, UP, UPLEFT, RIGHT, LEFT, DOWNRIGHT, DOWN, DOWNLEFT},
    {UPRIGHT, UP, UPLEFT, RIGHT, LEFT, DOWNRIGHT, DOWN, DOWNLEFT},
};

static void add_move(Move moves[], int count, char start, char end, char flag) {
    Move move = {start, end, flag};
    moves[count] = move;

    /* For debugging */
    printf("%c%d %c%d\n", (start & 7) + 'a', (start >> 4) + 1, (end & 7) + 'a',
           (end >> 4) + 1);
}

/* Generates pseudo-legal moves (checks are not considered) */
int generate_moves(Board *board, Move *moves) {
    int start, end, i, count = 0;

    /* Iterates over all coordinates */
    for (start = A1; start <= H8; start++) {
        /* Only generates moves for the player to move */
        if (board->colors[start] == board->player) {
            /* Generates moves for each piece type */
            char piece = board->pieces[start];
            if (piece == PAWN) {
                /* Pawn direction and second square depend on color */
                int dir = UP, sq = 1;
                if (board->player == BLACK) {
                    dir = DOWN, sq = 6;
                }

                /* Checks that the next square is empty */
                end = start + dir;
                if (!board->colors[end]) {
                    /* Adds all promotion moves */
                    if (get_rank(start) == 7 - sq) {
                        for (i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                            add_move(moves, count++, start, end, i);
                        }
                    } else {
                        /* Adds move and double move if on special rank */
                        add_move(moves, count++, start, end, NORMAL);
                        if (get_rank(start) == sq & !board->colors[end + dir]) {
                            add_move(moves, count++, start, end + dir, NORMAL);
                        }
                    }
                }

                /* Adds pawn attacks to the right and left and en passant */
                if (board->colors[end + RIGHT] != board->player &&
                    (board->colors[end + RIGHT] != EMPTY_COLOR ||
                     board->colors[end + RIGHT] == board->enpassant)) {
                    add_move(moves, count++, start, end + RIGHT, NORMAL);
                }
                if (board->colors[end + LEFT] != board->player &&
                    (board->colors[end + LEFT] != EMPTY_COLOR ||
                     board->colors[end + LEFT] == board->enpassant)) {
                    add_move(moves, count++, start, end + LEFT, NORMAL);
                }
            } else if (piece == KNIGHT || piece == KING) {
                /* Iterate over all directions*/
                for (i = 0; i < 8; i++) {
                    /* Adds move if target is not player to move's color */
                    end = start + vectors[piece - 2][i];
                    if (!invalid_square(end) &&
                        board->colors[end] != board->player) {
                        add_move(moves, count++, start, end, NORMAL);
                    }
                }
            } else {
                /* Number of directions to check */
                int directions = 8;
                if (piece == ROOK || piece == BISHOP) {
                    directions = 4;
                }
                for (i = 0; i < directions; i++) {
                    for (end = start + vectors[piece - 2][i];
                         !invalid_square(end); end += vectors[piece - 2][i]) {
                        /* Adds move if target is not own piece */
                        if (board->colors[end] != board->player) {
                            add_move(moves, count++, start, end, NORMAL);
                        }
                        /* Stops traversal once a piece is hit */
                        if (board->colors[end]) {
                            break;
                        }
                    }
                }
            }
        }
    }

    /* Add castling */

    return count;
}

/* Moves piece from start to end */
/* Assumes move is legal, so for player input moves, use legal_move first */
void move_piece(Board *board, int start, int end) {
    /* Updates board */
    board->colors[end] = board->colors[start];
    board->pieces[end] = board->pieces[start];
    board->colors[start] = EMPTY_COLOR;
    board->pieces[start] = EMPTY_PIECE;

    board->player = board->player == WHITE ? BLACK : WHITE;
}

/* Moves piece from start to end if it is legal */
int player_move_piece(Board *board, int start, int end) {
    Move moves[MAX_MOVES];
    int count, i;

    /* Start and end must be in the board and be different colors */
    /* Starting square must be the player to move's piece */
    if (invalid_square(start) || invalid_square(end) ||
        (board->colors[start] == board->colors[end]) ||
        board->colors[start] != board->player) {
        return FAILURE;
    }

    /* Check if piece is in moves list */
    count = generate_moves(board, moves);

    /* NOTE: Could change moves from array to hashset later, but this code is
    for checking if player moves are legal, so performance is not critical */

    /* Iterates through all legal moves and checks if the move is in there */
    for (i = 0; i < count; i++) {
        if (moves[i].start == start && moves[i].end == end) {
            /* TODO: check if moving the piece leaves the king in check */
            move_piece(board, start, end);
            return SUCCESS;
        }
    }

    return FAILURE;
}