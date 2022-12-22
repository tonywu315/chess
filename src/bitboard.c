#include "bitboard.h"
#include "move.h"
#include "move_generation.h"
#include "transposition.h"

// Initialize board struct to create an empty board
void init_board(Board *board) {
    *board = (const Board){0};

    for (int square = A1; square <= H8; square++) {
        board->board[square] = NO_PIECE;
    }

    board->player = WHITE;
}

// Print the chess board
void print_board(const Board *board, int score, bool game_over) {
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
    if (!game_over) {
        // Prints mate in x if detected
        if (is_mate_score(score)) {
            printf("Mate in %d\n", score_to_mate(score));
        } else {
            printf("Evaluation: %d\n", score);
        }

        // Print player to move
        printf("Player to move: %s\n",
               board->player == WHITE ? "White" : "Black");
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
    State state;
    int index = 0;

    init_board(board);
    state.capture = NO_PIECE;

    // Add each piece depending on character
    int square = A8;
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

    while (fen[++index] == ' ') {
    }

    // Set current player
    board->player = fen[index] == 'w' ? WHITE : BLACK;

    while (fen[++index] == ' ') {
    }

    // Set castle rights
    state.castling = 0;
    do {
        switch (fen[index]) {
        case 'K':
            state.castling |= CASTLE_WK;
            break;
        case 'Q':
            state.castling |= CASTLE_WQ;
            break;
        case 'k':
            state.castling |= CASTLE_BK;
            break;
        case 'q':
            state.castling |= CASTLE_BQ;
            break;
        }
    } while (fen[++index] != ' ');

    while (fen[++index] == ' ') {
    }

    // Set enpassant square
    if (fen[index] == '-') {
        state.enpassant = NO_SQUARE;
    } else {
        state.enpassant = make_square(fen[index] - 'a', (fen[index + 1] - '1'));
        index++;
    }

    while (fen[++index] == ' ') {
    }

    // Get halfmove clock
    state.draw_ply = 0;
    do {
        state.draw_ply = 10 * state.draw_ply + fen[index] - '0';
    } while (fen[++index] != ' ');

    while (fen[++index] == ' ') {
    }

    // Get fullmove counter
    int fullmove = 0;
    do {
        fullmove = fullmove * 10 + fen[index] - '0';
        index++;
    } while (fen[index] >= '0' && fen[index] <= '9');

    // Convert fullmove to ply
    board->ply = (fullmove - 1) * 2;
    if (board->player == BLACK) {
        board->ply++;
    }
    game.ply = board->ply;

    // Add pieces to occupancy bitboards
    for (int piece = PAWN; piece <= KING; piece++) {
        board->occupancies[0] |= board->pieces[piece];
        board->occupancies[1] |= board->pieces[piece + 8];
    }
    board->occupancies[2] = board->occupancies[0] | board->occupancies[1];

    board->state[board->ply] = state;
    board->hash = get_hash(board);
}
