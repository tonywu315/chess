#include "bitboard.h"
#include "move.h"
#include "move_generation.h"
#include "transposition.h"

// Initialize board struct to create an empty board
void init_board(Board *board) {
    State state = {NO_PIECE, 15, NO_SQUARE, 0};

    board->state[0] = state;
    memset(board->pieces, 0, sizeof(board->pieces));
    memset(board->occupancies, 0, sizeof(board->occupancies));

    for (int square = A1; square <= H8; square++) {
        board->board[square] = NO_PIECE;
    }

    board->hash = 0;
    board->player = WHITE;
    board->ply = 0;
}

// Load starting chess position
void start_board(Board *board) {
    init_board(board);
    load_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    board->hash = get_hash(board);
}

// Print the chess board
void print_board(const Board *board, int score) {
    const char pieces[] = {
        [W_PAWN] = 'P',   [W_KNIGHT] = 'N', [W_BISHOP] = 'B', [W_ROOK] = 'R',
        [W_QUEEN] = 'Q',  [W_KING] = 'K',   [B_PAWN] = 'p',   [B_KNIGHT] = 'n',
        [B_BISHOP] = 'b', [B_ROOK] = 'r',   [B_QUEEN] = 'q',  [B_KING] = 'k',
        [NO_PIECE] = ' ',
    };

    // Iterate through each row and file
    for (int i = 0; i < 8; i++) {
        int rank = board->player == WHITE ? 7 - i : i;

        printf("\n%d  ", rank + 1);
        for (int j = 0; j < 8; j++) {
            int file = board->player == WHITE ? j : 7 - j;
            int square = make_square(file, rank);

            // Print letter piece
            printf("%c ", pieces[board->board[square]]);
        }
    }

    // Print files at the bottom
    if (board->player == WHITE) {
        printf("\n\n   a b c d e f g h\n\n");
    } else {
        printf("\n\n   h g f e d c b a\n\n");
    }

    // Print score unless end of game
    if (abs(score) != INFINITY) {
        // Prints mate in x if detected
        if (abs(score) >= INFINITY - 100) {
            printf("Mate in %d\n", (INFINITY - abs(score)) / 2);
        } else {
            printf("Evaluation: %d\n", score);
        }

        // Print player to move
        printf("Player to move: %s\n",
               board->player == WHITE ? "White" : "Black");
    }

    // Print extra information for debugging
    if (DEBUG_FLAG) {
        State state = board->state[board->ply];
        printf("\nScore: %d\n", score);
        printf("Castling: %c%c%c%c\n", state.castling & CASTLE_WK ? 'K' : '-',
               state.castling & CASTLE_WQ ? 'Q' : '-',
               state.castling & CASTLE_BK ? 'k' : '-',
               state.castling & CASTLE_BQ ? 'q' : '-');
        printf("Enpassant: %s\n\n", state.enpassant != NO_SQUARE
                                        ? get_coordinates(state.enpassant)
                                        : "none");
    }
}

// Print bitboard in an 8x8 square
void print_bitboard(Bitboard bitboard) {
    // Print hex representation
    printf("\nBitboard: 0x%lX\n\n", bitboard);

    // Iterate through each rank and file
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            printf(" %c",
                   get_bit(bitboard, make_square(file, rank)) ? '1' : '.');
        }

        printf("\n");
    }

    // Print files at the bottom
    printf("\n   ");
    for (int i = 0; i < 8; i++) {
        printf("%c ", 'a' + i);
    }
    printf("\n\n");
}

// Load chess position from Forsyth-Edwards Notation
void load_fen(Board *board, const char *fen) {
    const unsigned char pieces[] = {
        ['P'] = W_PAWN,   ['N'] = W_KNIGHT, ['B'] = W_BISHOP, ['R'] = W_ROOK,
        ['Q'] = W_QUEEN,  ['K'] = W_KING,   ['p'] = B_PAWN,   ['n'] = B_KNIGHT,
        ['b'] = B_BISHOP, ['r'] = B_ROOK,   ['q'] = B_QUEEN,  ['k'] = B_KING,
    };
    int index = 0, square = A8;

    // Load empty board
    init_board(board);

    // Add each piece depending on character
    do {
        if (isalpha(fen[index])) {
            int piece = pieces[(int)fen[index]];
            board->board[square] = piece;
            set_bit(&board->pieces[piece], square++);
        } else if (fen[index] == '/') {
            square -= 16;
        } else {
            square += fen[index] - '0';
        }
    } while (fen[++index] != ' ');

    // Set current player
    board->player = fen[++index] == 'w' ? WHITE : BLACK;
    index += 2;

    // Set castle rights
    board->state[0].castling = 0;
    do {
        switch (fen[index]) {
        case 'K':
            board->state[0].castling |= CASTLE_WK;
            break;
        case 'Q':
            board->state[0].castling |= CASTLE_WQ;
            break;
        case 'k':
            board->state[0].castling |= CASTLE_BK;
            break;
        case 'q':
            board->state[0].castling |= CASTLE_BQ;
            break;
        }
    } while (fen[++index] != ' ');

    // Set enpassant square
    if (fen[++index] == '-') {
        index += 2;
    } else {
        board->state[0].enpassant =
            make_square(fen[index] - 'a', (fen[index + 1] - '1'));
        index += 3;
    }

    // Set halfmove clock for 50 move draw rule
    board->state[0].draw_ply = fen[index] - '0';

    // Add pieces to occupancy bitboards
    for (int piece = PAWN; piece <= KING; piece++) {
        board->occupancies[0] |= board->pieces[piece];
        board->occupancies[1] |= board->pieces[piece + 8];
    }
    board->occupancies[2] = board->occupancies[0] | board->occupancies[1];

    board->hash = get_hash(board);
}
