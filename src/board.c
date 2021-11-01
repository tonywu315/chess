#include "board.h"

/* Returns true if square is outside the board */
inline int invalid_square(Fast square) { return square & 0x88; }

/* Returns rank and file of square (number from 0 to 7) */
inline int get_rank(Fast square) { return square >> 4; }
inline int get_file(Fast square) { return square & 7; }

/* Initializes empty board */
void init_board() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        board.pieces[i] = EMPTY_PIECE;
        board.colors[i] = EMPTY_COLOR;
    }

    board.player = WHITE;
    board.castle = ALL_CASTLE;
    board.enpassant = 255;
    board.ply = 0;
    board.king[WHITE - 1] = 255;

    root_pos = 0;
    search_pos = 0;
}

/* Creates the starting board */
void start_board() {
    init_board(board);

    /* Starting chess pieces */
    board.pieces[A1] = ROOK;
    board.pieces[B1] = KNIGHT;
    board.pieces[C1] = BISHOP;
    board.pieces[D1] = QUEEN;
    board.pieces[E1] = KING;
    board.pieces[F1] = BISHOP;
    board.pieces[G1] = KNIGHT;
    board.pieces[H1] = ROOK;

    board.pieces[A8] = ROOK;
    board.pieces[B8] = KNIGHT;
    board.pieces[C8] = BISHOP;
    board.pieces[D8] = QUEEN;
    board.pieces[E8] = KING;
    board.pieces[F8] = BISHOP;
    board.pieces[G8] = KNIGHT;
    board.pieces[H8] = ROOK;

    board.king[WHITE - 1] = E1;
    board.king[BLACK - 1] = E8;

    /* Pawns */
    for (int i = A2; i <= H2; i++) {
        board.pieces[i] = PAWN;
    }
    for (int i = A7; i <= H7; i++) {
        board.pieces[i] = PAWN;
    }

    /* Colors for pieces */
    for (int i = A1; i <= H1; i++) {
        board.colors[i] = WHITE;
    }
    for (int i = A2; i <= H2; i++) {
        board.colors[i] = WHITE;
    }
    for (int i = A7; i <= H7; i++) {
        board.colors[i] = BLACK;
    }
    for (int i = A8; i <= H8; i++) {
        board.colors[i] = BLACK;
    }
}

/* Prints board in simple text format */
void print_board(int score) {
    char text[3][7] = {{' ', ' ', ' ', ' ', ' ', ' ', ' '},
                       {' ', 'P', 'N', 'B', 'R', 'Q', 'K'},
                       {' ', 'p', 'n', 'b', 'r', 'q', 'k'}};
    char players[3][6] = {"", "White", "Black"};

    printf("\n");

    /* Flips board if player is black */
    if (board.player == WHITE) {
        for (int i = 7; i >= 0; i--) {
            for (int j = 0; j < 8; j++) {
                int k = i * 16 + j;
                printf("%c", text[(int)board.colors[k]][(int)board.pieces[k]]);
            }
            printf("\n");
        }
    } else {
        for (int i = 0; i < 8; i++) {
            for (int j = 7; j >= 0; j--) {
                int k = i * 16 + j;
                printf("%c", text[(int)board.colors[k]][(int)board.pieces[k]]);
            }
            printf("\n");
        }
    }

    printf("\nEvaluation: %d\n", score);
    printf("Player to move: %s\n", players[(int)board.player]);
}