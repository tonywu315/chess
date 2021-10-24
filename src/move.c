#include "move.h"
#include "board.h"
#include <stdio.h>

static const char vectors[5][8] = {
    {33, 31, 18, 14, -14, -18, -31, -33},
    {UPRIGHT, UPLEFT, DOWNRIGHT, DOWNLEFT},
    {UP, RIGHT, LEFT, DOWN},
    {UPRIGHT, UP, UPLEFT, RIGHT, LEFT, DOWNRIGHT, DOWN, DOWNLEFT},
    {UPRIGHT, UP, UPLEFT, RIGHT, LEFT, DOWNRIGHT, DOWN, DOWNLEFT},
};

/* Returns true if square is outside the board */
/* Uses special property of 0x88 boards */
static int invalid_square(int square) { return square & 0x88; }

static void add_move(Move moves[], int count, int start, int end) {
    Move move = {start, end};
    moves[count] = move;
    printf("%c%d %c%d\n", (start & 7) + 'a', (start >> 4) + 1, (end & 7) + 'a',
           (end >> 4) + 1);
}

/* Generates pseudo-legal moves (checks are not considered) */
int generate_moves(Board *board, Move moves[]) {
    int start, i, j, count = 0;

    /* Iterates over all coordinates */
    for (start = A1; start <= H8; start++) {
        /* Only generates moves for the player to move */
        if (board->colors[start] == board->player) {
            /* Generates moves for each piece type */
            char piece = board->pieces[start];
            if (piece == PAWN) {
                /* TODO: Implement */
                ;
            } else {
                /* Number of directions to check */
                int directions = 8;
                if (piece == ROOK || piece == BISHOP) {
                    directions = 4;
                }

                /* Iterate over all directions */
                for (i = 0; i < directions; i++) {
                    int dir = vectors[piece - 2][i];
                    for (j = start + dir; !invalid_square(j); j += dir) {
                        /* Adds move if piece captures or moves to empty spot */
                        if (!board->colors[j]) {
                            add_move(moves, count++, start, j);
                        } else {
                            /* Stops ray traversal once capture is found */
                            if (board->colors[j] != board->player) {
                                add_move(moves, count++, start, j);
                            }
                            break;
                        }

                        /* Stops ray traversal if piece cannot slide */
                        if (piece == KNIGHT || piece == KING) {
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
    Move moves[MAXIMUM_MOVES];
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