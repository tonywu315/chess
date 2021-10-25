#include "board.h"
#include "constants.h"
#include "move.h"

static const char vectors[5][8] = {
    {33, 31, 18, 14, -14, -18, -31, -33},
    {UPRIGHT, UPLEFT, DOWNRIGHT, DOWNLEFT},
    {UP, RIGHT, LEFT, DOWN},
    {UPRIGHT, UP, UPLEFT, RIGHT, LEFT, DOWNRIGHT, DOWN, DOWNLEFT},
    {UPRIGHT, UP, UPLEFT, RIGHT, LEFT, DOWNRIGHT, DOWN, DOWNLEFT},
};

/* Creates a move with move information and current board information */
Move create_move(char start, char end, char flag) {
    Move move;
    move.start = start;
    move.end = end;
    move.flag = flag;
    move.captured = EMPTY_PIECE;
    move.enpassant = board.enpassant;
    move.ply = board.ply;

    if (board.colors[move.end] == 3 - board.player) {
        move.captured = board.pieces[move.end];
    }

    return move;
}

/* Adds the move to the moves list */
static void add_move(Move moves[], int count, char start, char end, char flag) {
    moves[count] = create_move(start, end, flag);

    /* For debugging */
    if (FALSE) {
        if (flag == CASTLE_WK || flag == CASTLE_BK) {
            printf("0-0\n");
        } else if (flag == CASTLE_WQ || flag == CASTLE_BQ) {
            printf("0-0-0\n");
        } else {
            printf("%c%d %c%d\n", (start & 7) + 'a', (start >> 4) + 1,
                   (end & 7) + 'a', (end >> 4) + 1);
        }
    }
}

/* Checks if opponent of player has a piece at square */
static int is_enemy(int square, char player, char piece) {
    return !invalid_square(square) && board.pieces[square] == piece &&
           board.colors[square] == 3 - player;
}

/* Checks if a square is attacked by the opponent of the player */
int is_attacked(int square, char player) {
    char piece;
    int i, attacker;

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

    /* Iterates over all coordinates */
    for (start = A1; start <= H8; start++) {
        /* Only generates moves for the player to move */
        if (board.colors[start] == board.player) {
            /* Generates moves for each piece type */
            char piece = board.pieces[start];
            if (piece == PAWN) {
                /* Pawn direction and second square depend on color */
                int direction = UP, second = 1;
                if (board.player == BLACK) {
                    direction = DOWN, second = 6;
                }

                /* Checks that the next square is empty */
                end = start + direction;
                if (!board.colors[end]) {
                    /* Adds all promotion moves */
                    if (get_rank(start) == 7 - second) {
                        for (i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                            add_move(moves, count++, start, end, i);
                        }

                        /* Adds promotion capture moves */
                        if (!invalid_square(end + RIGHT) &&
                            board.colors[end + RIGHT] == 3 - board.player) {
                            for (i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                                add_move(moves, count++, start, end + RIGHT, i);
                            }
                        }
                        if (!invalid_square(end + LEFT) &&
                            board.colors[end + RIGHT] == 3 - board.player) {
                            for (i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                                add_move(moves, count++, start, end + LEFT, i);
                            }
                        }

                    } else {
                        /* Adds move and double move if on special rank */
                        add_move(moves, count++, start, end, NORMAL);
                        if (get_rank(start) == second &&
                            !board.colors[end + direction]) {
                            add_move(moves, count++, start, end + direction,
                                     DOUBLE);
                        }
                    }
                }

                /* Adds pawn attacks to the right and left and en passant */
                i = end + RIGHT;
                if (!invalid_square(i) && board.colors[i] != board.player) {
                    if (board.colors[i] != EMPTY_COLOR) {
                        add_move(moves, count++, start, i, CAPTURE);
                    } else if (i == board.enpassant) {
                        add_move(moves, count++, start, i, ENPASSANT);
                    }
                }
                i = end + LEFT;
                if (!invalid_square(i) && board.colors[i] != board.player) {
                    if (board.colors[i] != EMPTY_COLOR) {
                        add_move(moves, count++, start, i, CAPTURE);
                    } else if (i == board.enpassant) {
                        add_move(moves, count++, start, i, ENPASSANT);
                    }
                }
            } else if (piece == KNIGHT || piece == KING) {
                /* Iterate over all directions*/
                for (i = 0; i < 8; i++) {
                    /* Adds normal move or capture move */
                    end = start + vectors[piece - 2][i];
                    if (!invalid_square(end)) {
                        if (board.colors[end] == EMPTY_COLOR) {
                            add_move(moves, count++, start, end, NORMAL);
                        } else if (board.colors[end] == 3 - board.player) {
                            add_move(moves, count++, start, end, CAPTURE);
                        }
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
                        /* Adds normal move or capture move */
                        if (board.colors[end] == EMPTY_COLOR) {
                            add_move(moves, count++, start, end, NORMAL);
                        } else if (board.colors[end] == 3 - board.player) {
                            add_move(moves, count++, start, end, CAPTURE);
                        }
                        /* Stops traversal once a piece is hit*/
                        if (board.colors[end]) {
                            break;
                        }
                    }
                }
            }
        }
    }

    return count;
}