#include "board.h"
#include <stdio.h>
#include <stdlib.h>

/* Initializes empty board */
void init_board(Board *board) {
    if (!board) {
        fprintf(stderr, "Error initializing board\n");
        exit(0);
    }

    int i;

    for (i = 0; i < ARRAY_SIZE; i++) {
        board->pieces[i] = EMPTY_PIECE;
        board->colors[i] = EMPTY_COLOR;
    }

    board->player = WHITE;
    board->enpassant = 0;
    board->castle = ALL_CASTLE;
    board->ply = 0;
}

/* Creates the starting board */
void start_board(Board *board) {
    if (!board) {
        fprintf(stderr, "Error creating starting board\n");
        exit(0);
    }

    int i;

    init_board(board);

    /* Starting chess pieces */
    board->pieces[A1] = ROOK;
    board->pieces[B1] = KNIGHT;
    board->pieces[C1] = BISHOP;
    board->pieces[D1] = QUEEN;
    board->pieces[E1] = KING;
    board->pieces[F1] = BISHOP;
    board->pieces[G1] = KNIGHT;
    board->pieces[H1] = ROOK;

    board->pieces[A8] = ROOK;
    board->pieces[B8] = KNIGHT;
    board->pieces[C8] = BISHOP;
    board->pieces[D8] = QUEEN;
    board->pieces[E8] = KING;
    board->pieces[F8] = BISHOP;
    board->pieces[G8] = KNIGHT;
    board->pieces[H8] = ROOK;

    /* Pawns */
    for (i = A2; i <= H2; i++) {
        board->pieces[i] = PAWN;
    }
    for (i = A7; i <= H7; i++) {
        board->pieces[i] = PAWN;
    }

    /* Colors for pieces */
    for (i = A1; i <= H1; i++) {
        board->colors[i] = WHITE;
    }
    for (i = A2; i <= H2; i++) {
        board->colors[i] = WHITE;
    }
    for (i = A7; i <= H7; i++) {
        board->colors[i] = BLACK;
    }
    for (i = A8; i <= H8; i++) {
        board->colors[i] = BLACK;
    }
}

/* Returns true if square is outside the board */
/* Uses special property of 0x88 boards */
int invalid_square(int square) { return square & 0x88; }

/* Moves piece from start to end if it is a legal move */
int move_piece(Board *board, int start, int end) {
    if (!board) {
        fprintf(stderr, "Error initializing board\n");
        exit(0);
    }

    /* Start and end must be in the board and be different colors */
    if (invalid_square(start) || invalid_square(end) ||
        (board->colors[start] == board->colors[end])) {
        return FAILURE;
    }

    /* TODO: Check for legal moves */

    /* Updates board */
    board->colors[end] = board->colors[start];
    board->pieces[end] = board->pieces[start];
    board->colors[start] = EMPTY_COLOR;
    board->pieces[start] = EMPTY_PIECE;

    board->player = board->player == WHITE ? BLACK : WHITE;

    return SUCCESS;
}

/* Prints board in simple text format */
void print_board(Board *board) {
    int i;
    char text_pieces[3][7] = {{' ', ' ', ' ', ' ', ' ', ' ', ' '},
                              {' ', 'P', 'N', 'B', 'R', 'Q', 'K'},
                              {' ', 'p', 'n', 'b', 'r', 'q', 'k'}};
    char players[3][6] = {"", "White", "Black"};

    if (!board) {
        fprintf(stderr, "Error printing board\n");
        exit(0);
    }

    /* Flips board if player is black */
    if (board->player == WHITE) {
        for (i = A8; i >= A1; i++) {
            if (i % 16 <= 7) {
                printf("%c", text_pieces[board->colors[i]][board->pieces[i]]);
                if (i % 16 == 7) {
                    printf("\n");
                }
            }
            if (i % 16 == 7) {
                i -= 24;
            }
        }
    } else {
        for (i = A1; i <= H8; i++) {
            if (i % 16 <= 7) {
                printf("%c", text_pieces[board->colors[i]][board->pieces[i]]);
                if (i % 16 == 7) {
                    printf("\n");
                }
            }
        }
    }

    printf("\nPlayer to move: %s\n", players[board->player]);
}

int main() {
    Board board;

    start_board(&board);
    print_board(&board);

    move_piece(&board, E2, E4);
    print_board(&board);
}