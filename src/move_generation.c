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

static int generate_pawn_move(Move *moves, int count, U8 start);
static int generate_piece_move(Move *moves, int count, U8 start, U8 piece);
static inline int exists(U8 square, U8 player, U8 piece);

/* Creates a move with move information and current board information */
void create_move(Move *move, U8 start, U8 end, U8 flag) {
    move->start = start;
    move->end = end;
    move->captured = EMPTY_PIECE;
    move->flag = flag;
    move->castle = board.castle;
    move->enpassant = board.enpassant;
    move->ply = board.ply;

    if (board.colors[move->end] == 3 - board.player) {
        move->captured = board.pieces[move->end];
    }
}

/* Checks if player is attacking square */
int is_attacking(U8 square, U8 player) {
    /* Returns false if square is invalid */
    if (invalid_square(square)) {
        return false;
    }

    /* Checks if pawn is attacking */
    int back = player == WHITE ? DOWN : UP;
    if ((!invalid_square(square + back + RIGHT) &&
         exists(square + back + RIGHT, player, PAWN)) ||
        (!invalid_square(square + back + LEFT) &&
         exists(square + back + LEFT, player, PAWN))) {
        return true;
    }

    for (int i = 0; i < 8; i++) {
        /* Checks if knight or king is attacking */
        if ((!invalid_square(square + vectors[KNIGHT - 2][i]) &&
             exists(square + vectors[KNIGHT - 2][i], player, KNIGHT)) ||
            (!invalid_square(square + vectors[KING - 2][i]) &&
             exists(square + vectors[KING - 2][i], player, KING))) {
            return true;
        }

        /* Checks if queen is attacking */
        for (U8 end = square + vectors[QUEEN - 2][i]; !invalid_square(end);
             end += vectors[QUEEN - 2][i]) {
            if (exists(end, player, QUEEN)) {
                return true;
            }
            if (board.colors[end]) {
                break;
            }
        }
    }

    /* Checks if rook or bishop are attacking */
    for (int i = 0; i < 4; i++) {
        for (U8 end = square + vectors[ROOK - 2][i]; !invalid_square(end);
             end += vectors[ROOK - 2][i]) {
            if (exists(end, player, ROOK)) {
                return true;
            }
            if (board.colors[end]) {
                break;
            }
        }
        for (U8 end = square + vectors[BISHOP - 2][i]; !invalid_square(end);
             end += vectors[BISHOP - 2][i]) {
            if (exists(end, player, BISHOP)) {
                return true;
            }
            if (board.colors[end]) {
                break;
            }
        }
    }

    return false;
}

/* After move, check if king is under attack */
int in_check() {
    return is_attacking(board.king[2 - board.player], board.player);
}

/* Generates pseudo-legal moves (checks are not considered) */
int generate_moves(Move *moves) {
    Move move;
    U8 enemy = 3 - board.player;
    int count = 0;

    /* Checks castle conditions for all 4 possible castling moves */
    if (board.player == WHITE) {
        if (board.castle & CASTLE_WK && board.pieces[F1] == EMPTY_PIECE &&
            board.pieces[G1] == EMPTY_PIECE && !is_attacking(E1, enemy) &&
            !is_attacking(F1, enemy) && !is_attacking(G1, enemy)) {
            create_move(&move, E1, G1, CASTLE_WK);
            moves[count++] = move;
        }
        if (board.castle & CASTLE_WQ && board.pieces[D1] == EMPTY_PIECE &&
            board.pieces[C1] == EMPTY_PIECE &&
            board.pieces[B1] == EMPTY_PIECE && !is_attacking(E1, enemy) &&
            !is_attacking(D1, enemy) && !is_attacking(C1, enemy)) {
            create_move(&move, E1, C1, CASTLE_WQ);
            moves[count++] = move;
        }
    } else {
        if (board.castle & CASTLE_BK && board.pieces[F8] == EMPTY_PIECE &&
            board.pieces[G8] == EMPTY_PIECE && !is_attacking(E8, enemy) &&
            !is_attacking(F8, enemy) && !is_attacking(G8, enemy)) {
            create_move(&move, E8, G8, CASTLE_BK);
            moves[count++] = move;
        }
        if (board.castle & CASTLE_BQ && board.pieces[D8] == EMPTY_PIECE &&
            board.pieces[C8] == EMPTY_PIECE &&
            board.pieces[B8] == EMPTY_PIECE && !is_attacking(E8, enemy) &&
            !is_attacking(D8, enemy) && !is_attacking(C8, enemy)) {
            create_move(&move, E8, C8, CASTLE_BQ);
            moves[count++] = move;
        }
    }

    /* Iterates over all coordinates */
    for (U8 start = A1; start <= H8; start++) {
        /* Only generates moves for the player to move */
        if (board.colors[start] == board.player) {
            /* Generates moves for each piece type */
            U8 piece = board.pieces[start];
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
static int generate_pawn_move(Move *moves, int count, U8 start) {
    Move move;

    /* Pawn direction and second square depend on color */
    int direction = UP, second = 1;
    if (board.player == BLACK) {
        direction = DOWN, second = 6;
    }

    /* Checks that the next square is empty */
    U8 end = start + direction;
    if (!board.colors[end]) {
        /* Adds all promotion moves */
        if (get_rank(start) == 7 - second) {
            for (int i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                create_move(&move, start, end, i);
                moves[count++] = move;
            }
        } else {
            /* Adds move and double move if on special rank */
            create_move(&move, start, end, NORMAL);
            moves[count++] = move;
            if (get_rank(start) == second && !board.colors[end + direction]) {
                create_move(&move, start, end + direction, DOUBLE);
                moves[count++] = move;
            }
        }
    }

    /* Adds pawn attacks to the right and left and en passant */
    U8 attack = end + RIGHT;
    if (!invalid_square(attack) && board.colors[attack] != board.player) {
        if (board.colors[attack] != EMPTY_COLOR) {
            /* Adds promotion capture moves */
            if (get_rank(start) == 7 - second) {
                for (int i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                    create_move(&move, start, attack, i);
                    moves[count++] = move;
                }
            } else {
                create_move(&move, start, attack, CAPTURE);
                moves[count++] = move;
            }
        } else if (attack == board.enpassant) {
            create_move(&move, start, attack, ENPASSANT);
            moves[count++] = move;
        }
    }
    attack = end + LEFT;
    if (!invalid_square(attack) && board.colors[attack] != board.player) {
        if (board.colors[attack] != EMPTY_COLOR) {
            /* Adds promotion capture moves */
            if (get_rank(start) == 7 - second) {
                for (int i = PROMOTION_N; i <= PROMOTION_Q; i++) {
                    create_move(&move, start, attack, i);
                    moves[count++] = move;
                }
            } else {
                create_move(&move, start, attack, CAPTURE);
                moves[count++] = move;
            }
        } else if (attack == board.enpassant) {
            create_move(&move, start, attack, ENPASSANT);
            moves[count++] = move;
        }
    }

    return count;
}

/* Generate moves for other pieces */
static int generate_piece_move(Move *moves, int count, U8 start, U8 piece) {
    Move move;

    /* Iterate over all directions*/
    if (piece == KNIGHT || piece == KING) {
        for (int i = 0; i < 8; i++) {
            /* Adds normal move or capture move */
            U8 end = start + vectors[piece - 2][i];
            if (!invalid_square(end)) {
                if (board.colors[end] == EMPTY_COLOR) {
                    create_move(&move, start, end, NORMAL);
                    moves[count++] = move;
                } else if (board.colors[end] == 3 - board.player) {
                    create_move(&move, start, end, CAPTURE);
                    moves[count++] = move;
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
            for (U8 end = start + vectors[piece - 2][i]; !invalid_square(end);
                 end += vectors[piece - 2][i]) {
                /* Adds normal move or capture move */
                if (board.colors[end] == EMPTY_COLOR) {
                    create_move(&move, start, end, NORMAL);
                    moves[count++] = move;
                } else if (board.colors[end] == 3 - board.player) {
                    create_move(&move, start, end, CAPTURE);
                    moves[count++] = move;
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

/* Checks if a certain player has a certain piece at square */
static inline int exists(U8 square, U8 player, U8 piece) {
    return board.pieces[square] == piece && board.colors[square] == player;
}