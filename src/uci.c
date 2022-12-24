#include "uci.h"
#include "board.h"
#include "move.h"
#include "search.h"

static inline bool parse_input(char *input);
static inline void parse_options(char *options);
static inline void parse_position(Board *board, char *position);
static inline void parse_go(Board *board, char *position);
static inline Move parse_move(Board *board, char *move);

// Start communication with GUI using the Universal Chess Interface
void start_uci(Board *board) {
    char input[MAX_LINE], *token_ptr;

    printf("Chess %s by Tony Wu\n", VERSION);

    while (true) {
        if (!parse_input(input)) {
            continue;
        }

        // Get first token
        char *token = strtok_r(input, " ", &token_ptr);

        // Ignore empty lines
        if (!token) {
            continue;
        }

        if (!strcmp(token, "uci")) {
            printf("id name Chess %s\n", VERSION);
            printf("id author Tony Wu\n");
            // TODO: options
            printf("uciok\n");
        } else if (!strcmp(token, "debug")) {
            ;
        } else if (!strcmp(token, "isready")) {
            printf("readyok\n");
        } else if (!strcmp(token, "setoption")) {
            parse_options(token_ptr);
        } else if (!strcmp(token, "ucinewgame")) {
            // TODO: wait until search is over then clear
            clear_search();
        } else if (!strcmp(token, "position")) {
            parse_position(board, token_ptr);
        } else if (!strcmp(token, "go")) {
            parse_go(board, token_ptr);
        } else if (!strcmp(token, "ponderhit")) {
            // TODO: implement pondering
        } else if (!strcmp(token, "stop")) {
            time_over = true;
        } else if (!strcmp(token, "quit")) {
            time_over = true;
            break;
        }

        // Debug commands (not part of UCI)
        else if (!strcmp(token, "print")) {
            print_board(board, 0, false);
        }
    }
}

// Parse input from stdin and return true if no errors
static inline bool parse_input(char *input) {
    // Read MAX_LINE - 1 characters from stdin
    if (!fgets(input, MAX_LINE, stdin)) {
        perror("error reading input");
        return false;
    }

    // Clear remaining characters from stdin and remove newline
    size_t length = strcspn(input, "\n");
    if (length == 10 - 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
        }
    }
    input[length] = '\0';

    return true;
}

static inline void parse_options(char *options) { ; }

// Load position from UCI command
static inline void parse_position(Board *board, char *position) {
    char *token = strtok_r(position, " ", &position);
    if (!token) {
        return;
    }

    // Split moves from position
    char *moves = strstr(position, "moves");
    if (moves) {
        if (moves[-1] != ' ' && moves[-1] != '\0') {
            return;
        }
        moves[-1] = '\0';
    }

    // Load position from FEN
    if (!strcmp(token, "startpos")) {
        if (!load_fen(board, START_FEN)) {
            return;
        }
    } else if (!strcmp(token, "fen")) {
        if (position == moves) {
            return;
        }

        // Remove quotes
        char *end = position + strlen(position) - 1;
        while (position[0] == ' ' || position[0] == '"') {
            if (*position++ == '"') {
                break;
            }
        }
        while (end[0] == ' ' || end[0] == '"') {
            if (*end-- == '"') {
                end[1] = '\0';
                break;
            }
        }

        if (!load_fen(board, position)) {
            return;
        }
    } else {
        return;
    }

    // Load moves
    if (moves) {
        Move move;
        token = strtok_r(moves, " ", &moves);

        // Play moves while legal
        while ((token = strtok_r(moves, " ", &moves)) &&
               (move = parse_move(board, token)) && move_legal(board, move)) {
        }
    }

    return;
}

static inline void parse_go(Board *board, char *position) {}

// Convert string to move
static inline Move parse_move(Board *board, char *move) {
    size_t length = strlen(move);

    // Check for special castling notation
    if (board->player == WHITE) {
        if (!strcmp(move, "O-O") || !strcmp(move, "0-0")) {
            return UINT16_C(0xF1C4);
        } else if (!strcmp(move, "O-O-O") || !strcmp(move, "0-0-0")) {
            return UINT16_C(0xF004);
        }
    } else {
        if (!strcmp(move, "O-O") || !strcmp(move, "0-0")) {
            return UINT16_C(0xFFFC);
        } else if (!strcmp(move, "O-O-O") || !strcmp(move, "0-0-0")) {
            return UINT16_C(0xFE3C);
        }
    }

    if (length != 4 && length != 5) {
        return NULL_MOVE;
    }

    // Get files and ranks in integer representation
    int start_file = tolower(move[0]) - 'a';
    int start_rank = move[1] - '1';
    int end_file = tolower(move[2]) - 'a';
    int end_rank = move[3] - '1';

    // Check that files and ranks are valid
    if (!(valid_row(start_file) && valid_row(start_rank) &&
          valid_row(end_file) && valid_row(end_rank))) {
        return NULL_MOVE;
    }

    // Create move
    int start = make_square(start_file, start_rank);
    int end = make_square(end_file, end_rank);

    // Check for castling
    if (board->board[start] == W_KING && start == E1 && end == G1) {
        return UINT16_C(0xF1C4);
    } else if (board->board[start] == W_KING && start == E1 && end == C1) {
        return UINT16_C(0xF004);
    } else if (board->board[start] == B_KING && start == E8 && end == G8) {
        return UINT16_C(0xFFFC);
    } else if (board->board[start] == B_KING && start == E8 && end == C8) {
        return UINT16_C(0xFE3C);
    }

    // Get promotion piece
    if (length == 5) {
        switch (tolower(move[4])) {
        case 'n':
            return encode_move(start, end, PROMOTION, KNIGHT);
        case 'b':
            return encode_move(start, end, PROMOTION, BISHOP);
        case 'r':
            return encode_move(start, end, PROMOTION, ROOK);
        case 'q':
            return encode_move(start, end, PROMOTION, QUEEN);
        }
        return NULL_MOVE;
    }
    return encode_move(start, end, 0, 0);
}
