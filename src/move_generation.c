#include "move_generation.h"
#include "board.h"
#include "move.h"

static const char vectors[5][8] = {
    {33, 31, 18, 14, -14, -18, -31, -33},
    {UPRIGHT, UPLEFT, DOWNRIGHT, DOWNLEFT},
    {UP, RIGHT, LEFT, DOWN},
    {UPRIGHT, UP, UPLEFT, RIGHT, LEFT, DOWNRIGHT, DOWN, DOWNLEFT},
    {UPRIGHT, UP, UPLEFT, RIGHT, LEFT, DOWNRIGHT, DOWN, DOWNLEFT},
};

static int generate_pawn_move(Move *moves, int count, Fast start);
static int generate_piece_move(Move *moves, int count, Fast start, Fast piece);
static int is_enemy(Fast square, Fast player, Fast piece);

/* Creates a move with move information and current board information */
Move create_move(Fast start, Fast end, Fast flag) {
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

/* Checks if a square is attacked by the opponent of the player */
int is_attacked(Fast square, Fast player) {
    /* Iterates through all the pieces */
    for (Fast piece = PAWN; piece <= KING; piece++) {
        if (piece == PAWN) {
            /* Checks if opponent pawn is attacking */
            int dir = player == WHITE ? UP : DOWN;
            if (is_enemy(square + dir + RIGHT, player, piece) ||
                is_enemy(square + dir + LEFT, player, piece)) {
                return true;
            }
        } else if (piece == KNIGHT || piece == KING) {
            /* Checks if opponent knight or king is attacking */
            for (int i = 0; i < 8; i++) {
                if (is_enemy(square + vectors[piece - 2][i], player, piece)) {
                    return true;
                }
            }
        } else {
            /* Number of directions to check */
            int directions = 8;
            if (piece == ROOK || piece == BISHOP) {
                directions = 4;
            }
            /* Checks if opponent sliding pieces are attacking */
            for (int i = 0; i < directions; i++) {
                Fast attacker = square;
                do {
                    attacker += vectors[piece - 2][i];
                    if (is_enemy(attacker, player, piece)) {
                        return true;
                    }
                } while (!invalid_square(attacker) && !board.colors[attacker]);
            }
        }
    }

    return false;
}

/* Generates pseudo-legal moves (checks are not considered) */
int generate_moves(Move *moves) {
    int count = 0;

    /* Checks castle conditions for all 4 possible castling moves */
    if (board.player == WHITE) {
        if (board.castle & CASTLE_WK && board.pieces[F1] == EMPTY_PIECE &&
            board.pieces[G1] == EMPTY_PIECE && !is_attacked(E1, board.player) &&
            !is_attacked(F1, board.player) && !is_attacked(G1, board.player)) {
            moves[count++] = create_move(E1, G1, CASTLE_WK);
        }
        if (board.castle & CASTLE_WQ && board.pieces[D1] == EMPTY_PIECE &&
            board.pieces[C1] == EMPTY_PIECE &&
            board.pieces[B1] == EMPTY_PIECE && !is_attacked(E1, board.player) &&
            !is_attacked(D1, board.player) && !is_attacked(C1, board.player)) {
            moves[count++] = create_move(E1, C1, CASTLE_WQ);
        }
    } else {
        if (board.castle & CASTLE_BK && board.pieces[F8] == EMPTY_PIECE &&
            board.pieces[G8] == EMPTY_PIECE && !is_attacked(E8, board.player) &&
            !is_attacked(F8, board.player) && !is_attacked(G8, board.player)) {
            moves[count++] = create_move(E8, G8, CASTLE_BK);
        }
        if (board.castle & CASTLE_BQ && board.pieces[D8] == EMPTY_PIECE &&
            board.pieces[C8] == EMPTY_PIECE &&
            board.pieces[B8] == EMPTY_PIECE && !is_attacked(E8, board.player) &&
            !is_attacked(D8, board.player) && !is_attacked(C8, board.player)) {
            moves[count++] = create_move(E8, C8, CASTLE_BQ);
        }
    }

    /* Iterates over all coordinates */
    for (Fast start = A1; start <= H8; start++) {
        /* Only generates moves for the player to move */
        if (board.colors[start] == board.player) {
            /* Generates moves for each piece type */
            Fast piece = board.pieces[start];
            if (piece == PAWN) {
                count = generate_pawn_move(moves, count, start);
            } else {
                count = generate_piece_move(moves, count, start, piece);
            }
        }
    }

    return count;
}

/* Generate pawn moves */
static int generate_pawn_move(Move *moves, int count, Fast start) {
    /* Pawn direction and second square depend on color */
    int direction = UP, second = 1;
    if (board.player == BLACK) {
        direction = DOWN, second = 6;
    }

    /* Checks that the next square is empty */
    Fast end = start + direction;
    if (!board.colors[end]) {
        /* Adds all promotion moves */
        if (get_rank(start) == 7 - second) {
            for (int i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                moves[count++] = create_move(start, end, i);
            }
        } else {
            /* Adds move and double move if on special rank */
            moves[count++] = create_move(start, end, NORMAL);
            if (get_rank(start) == second && !board.colors[end + direction]) {
                moves[count++] = create_move(start, end + direction, DOUBLE);
            }
        }
    }

    /* Adds pawn attacks to the right and left and en passant */
    Fast attack = end + RIGHT;
    if (!invalid_square(attack) && board.colors[attack] != board.player) {
        if (board.colors[attack] != EMPTY_COLOR) {
            /* Adds promotion capture moves */
            if (get_rank(start) == 7 - second) {
                for (int i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                    moves[count++] = create_move(start, attack, i);
                }
            } else {
                moves[count++] = create_move(start, attack, CAPTURE);
            }
        } else if (attack == board.enpassant) {
            moves[count++] = create_move(start, attack, ENPASSANT);
        }
    }
    attack = end + LEFT;
    if (!invalid_square(attack) && board.colors[attack] != board.player) {
        if (board.colors[attack] != EMPTY_COLOR) {
            /* Adds promotion capture moves */
            if (get_rank(start) == 7 - second) {
                for (int i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                    moves[count++] = create_move(start, attack, i);
                }
            } else {
                moves[count++] = create_move(start, attack, CAPTURE);
            }
        } else if (attack == board.enpassant) {
            moves[count++] = create_move(start, attack, ENPASSANT);
        }
    }

    return count;
}

/* Generate moves for other pieces */
static int generate_piece_move(Move *moves, int count, Fast start, Fast piece) {
    /* Iterate over all directions*/
    if (piece == KNIGHT || piece == KING) {
        for (int i = 0; i < 8; i++) {
            /* Adds normal move or capture move */
            Fast end = start + vectors[piece - 2][i];
            if (!invalid_square(end)) {
                if (board.colors[end] == EMPTY_COLOR) {
                    moves[count++] = create_move(start, end, NORMAL);
                } else if (board.colors[end] == 3 - board.player) {
                    moves[count++] = create_move(start, end, CAPTURE);
                }
            }
        }
    } else {
        /* Number of directions to check */
        int directions = 8;
        if (piece == ROOK || piece == BISHOP) {
            directions = 4;
        }
        for (int i = 0; i < directions; i++) {
            for (Fast end = start + vectors[piece - 2][i]; !invalid_square(end);
                 end += vectors[piece - 2][i]) {
                /* Adds normal move or capture move */
                if (board.colors[end] == EMPTY_COLOR) {
                    moves[count++] = create_move(start, end, NORMAL);
                } else if (board.colors[end] == 3 - board.player) {
                    moves[count++] = create_move(start, end, CAPTURE);
                }
                /* Stops traversal once a piece is hit*/
                if (board.colors[end]) {
                    break;
                }
            }
        }
    }

    return count;
}

/* Checks if opponent of player has a piece at square */
static int is_enemy(Fast square, Fast player, Fast piece) {
    return !invalid_square(square) && board.pieces[square] == piece &&
           board.colors[square] == 3 - player;
}