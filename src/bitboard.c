#include "bitboard.h"
#include "move.h"
#include "move_generation.h"

void init_board(Board *board) {
    memset(board->pieces, 0, sizeof(board->pieces));
    memset(board->occupancies, 0, sizeof(board->occupancies));

    board->player = WHITE;
    board->castle = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
    board->enpassant = NO_SQUARE;
    board->draw_ply = 0;
}

void start_board(Board *board) {
    init_board(board);
    load_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void print_board(Board board, int score) {
    const char pieces[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};

    printf("\n");

    for (int i = 0; i < 8; i++) {
        int rank = board.player == WHITE ? 7 - i : i;

        printf("%d  ", rank + 1);
        for (int j = 0; j < 8; j++) {
            int file = board.player == WHITE ? j : 7 - j;
            int square = rank * 8 + file;

            if (get_bit(board.occupancies[2], square)) {
                for (U8 piece = PAWN - 1; piece < KING; piece++) {
                    if (get_bit(board.pieces[piece], square)) {
                        printf("%c ", pieces[piece]);
                        break;
                    }
                    if (get_bit(board.pieces[piece + 6], square)) {
                        printf("%c ", tolower(pieces[piece]));
                        break;
                    }
                }
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
    if (board.player == WHITE) {
        printf("\n   a b c d e f g h\n\n");
    } else {
        printf("\n  h g f e d c b a\n\n");
    }

    /* Print score unless end of game */
    if (score != INT_MAX) {
        /* Prints mate in n if detected */
        if (abs(score) >= INT_MAX - 100) {
            printf("Mate in %d\n", (INT_MAX - abs(score)) / 2);
        } else {
            printf("Evaluation: %d\n", score);
        }

        printf("Player to move: %s\n", board.player - 1 ? "Black" : "White");
    }

    if (DEBUG_VALUE) {
        printf("\nCastling: %c%c%c%c\n", board.castle & CASTLE_WK ? 'K' : '-',
               board.castle & CASTLE_WQ ? 'Q' : '-',
               board.castle & CASTLE_BK ? 'k' : '-',
               board.castle & CASTLE_BQ ? 'q' : '-');
        printf("Enpassant: %s\n", board.enpassant != NO_SQUARE
                                      ? get_coordinates(board.enpassant)
                                      : "none");
    }
}

void print_bitboard(Bitboard bitboard) {
    printf("\nBitboard: 0x%lX\n\n", bitboard);
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            printf(" %c", get_bit(bitboard, rank * 8 + file) ? '1' : '.');
        }

        printf("\n");
    }

    printf("\n   ");
    for (int i = 0; i < 8; i++) {
        printf("%c ", 'a' + i);
    }
    printf("\n\n");
}

void load_fen(Board *board, const char *fen) {
    int index = 0, square = A8;
    const unsigned char indices[] = {
        ['P'] = 0, ['N'] = 1, ['B'] = 2, ['R'] = 3, ['Q'] = 4,  ['K'] = 5,
        ['p'] = 6, ['n'] = 7, ['b'] = 8, ['r'] = 9, ['q'] = 10, ['k'] = 11};

    init_board(board);

    /* Adds piece depending on character */
    do {
        if (isalpha(fen[index])) {
            set_bit(&board->pieces[indices[(int)fen[index]]], square++);
        } else if (fen[index] == '/') {
            square -= 16;
        } else {
            square += fen[index] - '0';
        }
    } while (fen[++index] != ' ');

    /* Side to move */
    board->player = fen[++index] == 'w' ? WHITE : BLACK;
    index += 2;

    /* Castle rights */
    board->castle = 0;
    do {
        switch (fen[index]) {
        case 'K':
            board->castle |= CASTLE_WK;
            break;
        case 'Q':
            board->castle |= CASTLE_WQ;
            break;
        case 'k':
            board->castle |= CASTLE_BK;
            break;
        case 'q':
            board->castle |= CASTLE_BQ;
            break;
        }
    } while (fen[++index] != ' ');

    /* Enpassant square */
    if (fen[++index] == '-') {
        index += 2;
    } else {
        board->enpassant = fen[index] - 'a' + 8 * (fen[index + 1] - '1');
        index += 3;
    }

    board->draw_ply = fen[index] - '0';

    for (U8 piece = PAWN - 1; piece < KING; piece++) {
        board->occupancies[0] |= board->pieces[piece];
        board->occupancies[1] |= board->pieces[piece + 6];
    }
    board->occupancies[2] = board->occupancies[0] | board->occupancies[1];
}

char *get_coordinates(U8 square) {
    // clang-format off
    static char coordinates[64][3] = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    };
    // clang-format on
    return coordinates[square];
}
