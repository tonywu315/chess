#include "uci.h"
#include "benchmark.h"
#include "board.h"
#include "move.h"
#include "search.h"
#include "transposition.h"

Parameter parameter;

static inline bool parse_input(char *input);
static inline void parse_options(char *option);
static inline void parse_position(Board *board, char *position);
static inline void parse_go(Board *board, char *input);
static inline Move parse_move(Board *board, char *move);
static inline void trim_whitespace(char **input);
static inline void lowercase(char *input);

// Start communication with GUI using the Universal Chess Interface
void start_uci(Board *board) {
    char input[MAX_LINE], *token_ptr;

    printf("Chess %s by Tony Wu\n", VERSION);

    while (true) {
        if (!parse_input(input)) {
            continue;
        }

        // Get UCI command
        char *token = strtok_r(input, " \t", &token_ptr);
        if (!token) {
            continue;
        }

        if (!strcmp(token, "uci")) {
            printf("id name Chess %s\n", VERSION);
            printf("id author Tony Wu\n\n");

            printf("option name Hash"
                   " type spin default 512 min 1 max 1073741824\n");

            printf("\nuciok\n");
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
        else if (!strcmp(token, "board")) {
            print_board(board, 0, false);
        } else if (!strcmp(token, "perft")) {
            if ((token = strtok_r(token_ptr, " \t", &token_ptr))) {
                benchmark(board, atoi(token));
            } else {
                benchmark(board, 6);
            }
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

// Parse options from UCI command
static inline void parse_options(char *option) {
    char *token = strtok_r(option, " \t", &option);
    char *value = strstr(option, " value ");

    // Check that input is valid
    if (!token || strcmp(token, "name") || !value) {
        return;
    }
    value[0] = '\0';
    value += 7;

    trim_whitespace(&option);
    trim_whitespace(&value);
    lowercase(option);
    lowercase(value);

    if (!strcmp(option, "hash")) {
        init_transposition(atoi(value));
    }
}

// Parse position from UCI command
static inline void parse_position(Board *board, char *position) {
    char *token = strtok_r(position, " \t", &position);
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
        trim_whitespace(&position);
        size_t end = strlen(position) - 1;
        if (position[0] == '"' && position[end] == '"') {
            position[end] = '\0';
            position++;
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
        token = strtok_r(moves, " \t", &moves);

        // Play moves while legal
        while ((token = strtok_r(moves, " \t", &moves)) &&
               (move = parse_move(board, token)) != NULL_MOVE &&
               move_legal(board, move)) {
        }
    }

    return;
}

// Parse search parameters from UCI command
static inline void parse_go(Board *board, char *input) {
    char *token;

    // Reset search parameters
    parameter = (const Parameter){0};

    // Iterate through all tokens
    while ((token = strtok_r(input, " \t", &input))) {
        if (!strcmp(token, "searchmoves")) {
            Move move;
            while ((token = strtok_r(input, " \t", &input)) &&
                   (move = parse_move(board, token)) != NULL_MOVE) {
                if (move_legal(board, move)) {
                    unmake_move(board, move);
                    parameter.search_moves[parameter.move_count++] = move;
                }
            }
        }
        if (!token) {
            break;
        }

        if (!strcmp(token, "ponder")) {
            parameter.ponder = true;
        } else if (!strcmp(token, "wtime")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                return;
            }
            parameter.white_time = atoi(token);
        } else if (!strcmp(token, "btime")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                return;
            }
            parameter.black_time = atoi(token);
        } else if (!strcmp(token, "winc")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                return;
            }
            parameter.white_increment = atoi(token);
        } else if (!strcmp(token, "binc")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                return;
            }
            parameter.black_increment = atoi(token);
        } else if (!strcmp(token, "movestogo")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                return;
            }
            parameter.moves_to_go = atoi(token);
        } else if (!strcmp(token, "depth")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                return;
            }
            parameter.max_depth = atoi(token);
        } else if (!strcmp(token, "nodes")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                return;
            }
            parameter.max_nodes = atoi(token);
        } else if (!strcmp(token, "mate")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                return;
            }
            parameter.mate = atoi(token);
        } else if (!strcmp(token, "movetime")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                return;
            }
            parameter.move_time = atoi(token);
        } else if (!strcmp(token, "infinite")) {
            parameter.infinite = true;
        }
    }
}

// Parse move from UCI command and return move
static inline Move parse_move(Board *board, char *move) {
    size_t length = strlen(move);

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

// Trim whitespace from string in place
static inline void trim_whitespace(char **input) {
    // Trim leading space
    while (**input == ' ' || **input == '\t') {
        (*input)++;
    }

    // Trim trailing space
    char *end = *input + strlen(*input) - 1;
    while (end > *input && (*end == ' ' || *end == '\t')) {
        end--;
    }
    end[1] = '\0';
}

// Convert string to lowercase in place
static inline void lowercase(char *input) {
    for (; *input; input++) {
        *input = tolower(*input);
    }
}
