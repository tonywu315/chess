#include "board.h"
#include "attacks.h"
#include "move.h"
#include "move_generation.h"
#include "transposition.h"

static inline bool load_move_counters(Board *board, const char *halfmove,
                               const char *fullmove);
static inline bool load_player(Board *board, const char *player);
static inline bool load_pieces(Board *board, const char *pieces);
static inline bool load_castling(Board *board, const char *castling);
static inline bool load_enpassant(Board *board, const char *enpassant);
static inline int parse_integer(const char *str);

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
    const char piece_codes[] = {
        [W_PAWN] = 'P',   [W_KNIGHT] = 'N', [W_BISHOP] = 'B', [W_ROOK] = 'R',
        [W_QUEEN] = 'Q',  [W_KING] = 'K',   [B_PAWN] = 'p',   [B_KNIGHT] = 'n',
        [B_BISHOP] = 'b', [B_ROOK] = 'r',   [B_QUEEN] = 'q',  [B_KING] = 'k',
        [NO_PIECE] = ' ',
    };

    // Iterate through each rank and file
    for (int i = 0; i < 8; i++) {
        int rank = board->player == WHITE ? 7 - i : i;

        printf("\n%d  ", rank + 1);
        for (int j = 0; j < 8; j++) {
            int file = board->player == WHITE ? j : 7 - j;
            int square = make_square(file, rank);

            // Print letter piece
            printf("%c ", piece_codes[board->board[square]]);
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

// Validate Forsyth-Edwards Notation string and load board position if valid
bool load_fen(Board *board, const char *fen) {
    Board new_board = {0};
    char string[128] = "";
    char *pieces, *player, *castling, *enpassant, *halfmove, *fullmove;

    // Split FEN string into fields
    strncat(string, fen, sizeof(string) - 1);
    pieces = strtok(string, " ");
    player = strtok(NULL, " ");
    castling = strtok(NULL, " ");
    enpassant = strtok(NULL, " ");
    halfmove = strtok(NULL, " ");
    fullmove = strtok(NULL, " ");

    // Check that there are exactly 6 fields
    if (!fullmove || strtok(NULL, " ")) {
        return false;
    }

    if (!(load_move_counters(&new_board, halfmove, fullmove) &&
          load_player(&new_board, player) && load_pieces(&new_board, pieces) &&
          load_castling(&new_board, castling) &&
          load_enpassant(&new_board, enpassant))) {
        return false;
    }

    // Set board to created board
    *board = new_board;

    // Add pieces to occupancy bitboards
    for (int piece = PAWN; piece <= KING; piece++) {
        board->occupancies[0] |= board->pieces[piece];
        board->occupancies[1] |= board->pieces[piece + 8];
    }
    board->occupancies[2] = board->occupancies[0] | board->occupancies[1];

    board->state[board->ply].capture = NO_PIECE;
    board->hash = get_hash(board);
    game.ply = board->ply;

    return true;
}

// Load halfmove and fullmove counters from FEN string
static inline bool load_move_counters(Board *board, const char *halfmove,
                               const char *fullmove) {
    State state;

    // Get halfmove counter
    int halfmoves = parse_integer(halfmove);
    if (halfmoves == -1) {
        return false;
    }

    // Get fullmove counter
    int fullmoves = parse_integer(fullmove);
    if (fullmoves <= 0) {
        return false;
    }
    int ply = (fullmoves - 1) * 2 + (board->player == BLACK ? 1 : 0);

    // Halfmove clock must be less than or equal to ply
    if (halfmoves > ply) {
        return false;
    }

    state.draw_ply = halfmoves;
    board->state[ply] = state;

    return true;
}

// Load player from FEN string
static inline bool load_player(Board *board, const char *player) {
    if (!strcmp(player, "w")) {
        board->player = WHITE;
        return true;
    } else if (!strcmp(player, "b")) {
        board->player = BLACK;
        return true;
    }
    return false;
}

// Load pieces from FEN string
static inline bool load_pieces(Board *board, const char *pieces) {
    const unsigned char piece_codes[] = {
        ['P'] = W_PAWN,   ['N'] = W_KNIGHT, ['B'] = W_BISHOP, ['R'] = W_ROOK,
        ['Q'] = W_QUEEN,  ['K'] = W_KING,   ['p'] = B_PAWN,   ['n'] = B_KNIGHT,
        ['b'] = B_BISHOP, ['r'] = B_ROOK,   ['q'] = B_QUEEN,  ['k'] = B_KING,
    };
    int square = A8, rank = 0, file = 0;
    bool space = false;

    // Iterate over each character in FEN string
    for (size_t i = 0, n = strlen(pieces); i < n; i++) {
        char c = pieces[i];
        if (strchr("PNBRQKpnbrqk", c)) {
            int piece = piece_codes[(int)c];
            board->board[square] = piece;
            set_bit(&board->pieces[piece], square++);

            space = false;
            if (++file > 8) {
                return false;
            }
        } else if (c >= '1' && c <= '8') {
            if (space == true) {
                return false;
            }
            space = true;

            int shift = c - '0';
            if ((file += shift) > 8) {
                return false;
            }
            for (; shift > 0; shift--) {
                board->board[square++] = NO_PIECE;
            }
        } else if (c == '/') {
            if (file != 8) {
                return false;
            }

            space = false;
            square -= 16;
            file = 0;
            rank++;
        } else {
            return false;
        }
    }

    // Check that all squares are filled
    if (!(rank == 7 && file == 8 && square == A2)) {
        return false;
    }

    // Check that the promotion squares do not contain pawns
    if ((board->pieces[W_PAWN] & UINT64_C(0xFF00000000000000)) |
        (board->pieces[B_PAWN] & UINT64_C(0xFF))) {
        return false;
    }

    // Check that there is exactly one king of each color
    if (get_population(board->pieces[W_KING]) != 1 ||
        get_population(board->pieces[B_KING]) != 1 ||
        board->pieces[W_KING] == board->pieces[B_KING]) {
        return false;
    }

    // Check that the player to move is not checking the other player
    if (in_check(board, !board->player)) {
        return false;
    }

    return true;
}

// Load castling rights from FEN string
static inline bool load_castling(Board *board, const char *castling) {
    State *state = &board->state[board->ply];

    state->castling = 0;
    if (strcmp(castling, "-")) {
        // Check that castling is subsequence of KQkq
        size_t i = 0, j = 0, n = strlen(castling);
        while (i < n && j < 4) {
            if (castling[i] == "KQkq"[j++]) {
                i++;
            }
        }
        if (i != n) {
            return false;
        }

        // Set castling rights
        for (size_t i = 0; i < n; i++) {
            switch (castling[i]) {
            case 'K':
                if (board->board[E1] != W_KING || board->board[H1] != W_ROOK) {
                    return false;
                }
                state->castling |= CASTLE_WK;
                break;
            case 'Q':
                if (board->board[E1] != W_KING || board->board[A1] != W_ROOK) {
                    return false;
                }
                state->castling |= CASTLE_WQ;
                break;
            case 'k':
                if (board->board[E8] != B_KING || board->board[H8] != B_ROOK) {
                    return false;
                }
                state->castling |= CASTLE_BK;
                break;
            case 'q':
                if (board->board[E8] != B_KING || board->board[A8] != B_ROOK) {
                    return false;
                }
                state->castling |= CASTLE_BQ;
                break;
            }
        }
    }

    return true;
}

// Load enpassant square from FEN string
static inline bool load_enpassant(Board *board, const char *enpassant) {
    State *state = &board->state[board->ply];

    if (!strcmp(enpassant, "-")) {
        state->enpassant = NO_SQUARE;
        return true;
    }

    if (strlen(enpassant) != 2) {
        return false;
    }

    int file = enpassant[0] - 'a';
    int rank = enpassant[1] - '1';
    if (!(valid_row(file) && (rank == 2 || rank == 5))) {
        return false;
    }
    state->enpassant = make_square(file, rank);

    // Check that there is a pawn in front of enpassant square
    if (rank == 2) {
        if (board->board[state->enpassant + 8] != W_PAWN) {
            return false;
        }
    } else if (board->board[state->enpassant - 8] != B_PAWN) {
        return false;
    }

    // Halfmove must be 0
    if (state->draw_ply != 0) {
        return false;
    }

    return true;
}

// Convert string to integer and return -1 if invalid
static inline int parse_integer(const char *str) {
    int value = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return -1;
        }
        value = value * 10 + (str[i] - '0');
    }

    return value;
}
