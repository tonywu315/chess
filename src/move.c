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

/* Adds the move to the moves list */
static void add_move(Move moves[], int count, char start, char end, char flag) {
    Move move = {start, end, flag};
    moves[count] = move;

    /* For debugging */
    if (flag == CASTLE_WK || flag == CASTLE_BK) {
        printf("0-0\n");
    } else if (flag == CASTLE_WQ || flag == CASTLE_BQ) {
        printf("0-0-0\n");
    } else {
        printf("%c%d %c%d\n", (start & 7) + 'a', (start >> 4) + 1,
               (end & 7) + 'a', (end >> 4) + 1);
    }
}

/* Checks if opponent of player has a piece at square */
static int is_enemy(int square, char player, char piece) {
    return !invalid_square(square) && board.pieces[square] == piece &&
           board.colors[square] == 3 - player;
}

/* Checks if a square is attacked by the opponent of the player */
static int is_attacked(int square, char player) {
    char piece, attacker;
    int i;

    /* Iterates through all the pieces */
    for (piece = PAWN; piece <= KING; piece++) {
        if (piece == PAWN) {
            /* Checks if opponent pawn is attacking */
            int dir = player == WHITE ? UP : DOWN;
            if (is_enemy(square + dir + RIGHT, player, piece) ||
                is_enemy(square + dir + LEFT, player, piece)) {
                return TRUE;
            }
        } else if (piece == KNIGHT || piece == KING) {
            /* Checks if opponent knight or king is attacking */
            for (i = 0; i < 8; i++) {
                if (is_enemy(square + vectors[piece - 2][i], player, piece)) {
                    return TRUE;
                }
            }
        } else {
            /* Number of directions to check */
            int directions = 8;
            if (piece == ROOK || piece == BISHOP) {
                directions = 4;
            }
            /* Checks if opponent sliding pieces are attacking */
            for (i = 0; i < directions; i++) {
                attacker = square;
                do {
                    attacker += vectors[piece - 2][i];
                    if (is_enemy(attacker, player, piece)) {
                        return TRUE;
                    }
                } while (!invalid_square(attacker) && !board.colors[attacker]);
            }
        }
    }

    return FALSE;
}

/* Generates pseudo-legal moves (checks are not considered) */
int generate_moves(Move *moves) {
    int start, end, i, count = 0;

    /* Iterates over all coordinates */
    for (start = A1; start <= H8; start++) {
        /* Only generates moves for the player to move */
        if (board.colors[start] == board.player) {
            /* Generates moves for each piece type */
            char piece = board.pieces[start];
            if (piece == PAWN) {
                /* Pawn direction and second square depend on color */
                int dir = UP, sq = 1;
                if (board.player == BLACK) {
                    dir = DOWN, sq = 6;
                }

                /* Checks that the next square is empty */
                end = start + dir;
                if (!board.colors[end]) {
                    /* Adds all promotion moves */
                    if (get_rank(start) == 7 - sq) {
                        for (i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                            add_move(moves, count++, start, end, i);
                        }
                    } else {
                        /* Adds move and double move if on special rank */
                        add_move(moves, count++, start, end, NORMAL);
                        if (get_rank(start) == sq & !board.colors[end + dir]) {
                            add_move(moves, count++, start, end + dir, NORMAL);
                        }
                    }
                }

                /* Adds pawn attacks to the right and left and en passant */
                if (board.colors[end + RIGHT] != board.player &&
                    (board.colors[end + RIGHT] != EMPTY_COLOR ||
                     board.colors[end + RIGHT] == board.enpassant)) {
                    add_move(moves, count++, start, end + RIGHT, NORMAL);
                }
                if (board.colors[end + LEFT] != board.player &&
                    (board.colors[end + LEFT] != EMPTY_COLOR ||
                     board.colors[end + LEFT] == board.enpassant)) {
                    add_move(moves, count++, start, end + LEFT, NORMAL);
                }
            } else if (piece == KNIGHT || piece == KING) {
                /* Iterate over all directions*/
                for (i = 0; i < 8; i++) {
                    /* Adds move if target is not player to move's color */
                    end = start + vectors[piece - 2][i];
                    if (!invalid_square(end) &&
                        board.colors[end] != board.player) {
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
                        if (board.colors[end] != board.player) {
                            add_move(moves, count++, start, end, NORMAL);
                        }
                        /* Stops traversal once a piece is hit */
                        if (board.colors[end]) {
                            break;
                        }
                    }
                }
            }
        }
    }

    /* Checks castle conditions for all 4 possible castling moves */
    if (board.player == WHITE) {
        if (board.castle & CASTLE_WK && board.pieces[F1] == EMPTY_PIECE &&
            board.pieces[G1] == EMPTY_PIECE && !is_attacked(E1, board.player) &&
            !is_attacked(F1, board.player) && !is_attacked(G1, board.player)) {
            add_move(moves, count++, E1, G1, CASTLE_WK);
        }
        if (board.castle & CASTLE_WQ && board.pieces[D1] == EMPTY_PIECE &&
            board.pieces[C1] == EMPTY_PIECE &&
            board.pieces[B1] == EMPTY_PIECE && !is_attacked(E1, board.player) &&
            !is_attacked(D1, board.player) && !is_attacked(C1, board.player)) {
            add_move(moves, count++, E1, C1, CASTLE_WQ);
        }
    } else {
        if (board.castle & CASTLE_BK && board.pieces[F8] == EMPTY_PIECE &&
            board.pieces[G8] == EMPTY_PIECE && !is_attacked(E8, board.player) &&
            !is_attacked(F8, board.player) && !is_attacked(G8, board.player)) {
            add_move(moves, count++, E8, G8, CASTLE_BK);
        }
        if (board.castle & CASTLE_BQ && board.pieces[D8] == EMPTY_PIECE &&
            board.pieces[C8] == EMPTY_PIECE &&
            board.pieces[B8] == EMPTY_PIECE && !is_attacked(E8, board.player) &&
            !is_attacked(D8, board.player) && !is_attacked(C8, board.player)) {
            add_move(moves, count++, E8, C8, CASTLE_BQ);
        }
    }

    return count;
}

/* TODO: change move_piece to accept a Move struct */

/* Moves piece from start to end */
/* Assumes move is legal, so for player input moves, use legal_move first */
void move_piece(int start, int end) {
    /* Updates board */
    board.colors[end] = board.colors[start];
    board.pieces[end] = board.pieces[start];
    board.colors[start] = EMPTY_COLOR;
    board.pieces[start] = EMPTY_PIECE;

    board.player = 3 - board.player;
}


/* Moves piece from start to end if it is legal */
int player_move_piece(int start, int end) {
    Move moves[MAX_MOVES];
    int count, i;

    /* Start and end must be in the board and be different colors */
    /* Starting square must be the player to move's piece */
    if (invalid_square(start) || invalid_square(end) ||
        (board.colors[start] == board.colors[end]) ||
        board.colors[start] != board.player) {
        return FAILURE;
    }

    /* Check if piece is in moves list */
    count = generate_moves(moves);

    /* NOTE: Could change moves from array to hashset later, but this code is
    for checking if player moves are legal, so performance is not critical */

    /* Iterates through all legal moves and checks if the move is in there */
    for (i = 0; i < count; i++) {
        if (moves[i].start == start && moves[i].end == end) {
            /* TODO: check if moving the piece leaves the king in check */
            move_piece(start, end);
            return SUCCESS;
        }
    }

    return FAILURE;
}