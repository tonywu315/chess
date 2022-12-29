#include "uci.h"
#include "attacks.h"
#include "benchmark.h"
#include "board.h"
#include "evaluation.h"
#include "move.h"
#include "search.h"
#include "transposition.h"

typedef struct argument {
    Board *board;
    Parameter parameters;
} Argument;

typedef struct node {
    void (*function)(char *, Board *);
    char *input;
    struct node *next;
} Node;

typedef struct queue {
    Node *head;
    Node *tail;
} Queue;

Queue queue;
Argument argument;
pthread_t old_tid, search_tid;
bool idle = true;

static inline bool parse_input(char *input);
static inline void parse_option(char *option, __UNUSED__ Board *board);
static inline void parse_position(char *option, Board *board);
static inline void parse_go(char *option, Board *board);
static inline Move parse_move(char *move, Board *board);
static inline void trim_whitespace(char **input);
static inline void lowercase(char *input);

static void *init_all(void *board);
static void *search_thread();

static inline void enqueue(void (*function)(char *, Board *), char *input);
static inline Node *dequeue();

// Start communication with GUI using the Universal Chess Interface
void start_uci(Board *board) {
    char input[MAX_LINE], *token_ptr;
    pthread_t init_tid;

    printf("Chess %s by Tony Wu\n", VERSION);

    // Initialize internal data structures
    pthread_create(&init_tid, NULL, init_all, board);

    // UCI loop
    while (true) {
        if (!parse_input(input)) {
            continue;
        }

        // Get UCI command
        char *token = strtok_r(input, " \t", &token_ptr);
        if (!token) {
            continue;
        }

        // TODO: implement "debug" and "ponderhit"

        // Commands to run instantly
        if (!strcmp(token, "uci")) {
            printf("id name Chess %s\n", VERSION);
            printf("id author Tony Wu\n\n");

            printf("option name Hash"
                   " type spin default 512 min 1 max 1073741824\n");

            printf("\nuciok\n");
        } else if (!strcmp(token, "isready")) {
            if (init_tid) {
                pthread_join(init_tid, NULL);
                init_tid = 0;
            }
            printf("readyok\n");
        } else if (!strcmp(token, "stop")) {
            time_over = true;
        } else if (!strcmp(token, "quit")) {
            break;
        }

        // Commands to run after current search is finished
        else if (!strcmp(token, "setoption")) {
            if (idle) {
                parse_option(token_ptr, NULL);
            } else {
                enqueue(parse_option, token_ptr);
            }
        } else if (!strcmp(token, "ucinewgame")) {
            if (idle) {
                clear_transposition();
            } else {
                enqueue(clear_transposition, token_ptr);
            }
        } else if (!strcmp(token, "position")) {
            if (idle) {
                parse_position(token_ptr, board);
            } else {
                enqueue(parse_position, token_ptr);
            }
        } else if (!strcmp(token, "go")) {
            if (idle) {
                idle = false;
                parse_go(token_ptr, board);
            } else {
                enqueue(parse_go, token_ptr);
            }
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
static inline void parse_option(char *option, __UNUSED__ Board *board) {
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
static inline void parse_position(char *position, Board *board) {
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
               (move = parse_move(token, board)) != NULL_MOVE &&
               move_legal(board, move)) {
        }
    }
}

// Parse search parameters from UCI command
static inline void parse_go(char *input, Board *board) {
    char *token;
    Parameter parameters;

    // Iterate through all tokens
    while ((token = strtok_r(input, " \t", &input))) {
        if (!strcmp(token, "searchmoves")) {
            Move move;
            while ((token = strtok_r(input, " \t", &input)) &&
                   (move = parse_move(token, board)) != NULL_MOVE) {
                if (move_legal(board, move)) {
                    unmake_move(board, move);
                    parameters.search_moves[parameters.move_count++] = move;
                }
            }
        }
        if (!token) {
            break;
        }

        if (!strcmp(token, "ponder")) {
            parameters.ponder = true;
        } else if (!strcmp(token, "wtime")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                break;
            }
            parameters.white_time = atoi(token);
        } else if (!strcmp(token, "btime")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                break;
            }
            parameters.black_time = atoi(token);
        } else if (!strcmp(token, "winc")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                break;
            }
            parameters.white_increment = atoi(token);
        } else if (!strcmp(token, "binc")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                break;
            }
            parameters.black_increment = atoi(token);
        } else if (!strcmp(token, "movestogo")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                break;
            }
            parameters.moves_to_go = atoi(token);
        } else if (!strcmp(token, "depth")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                break;
            }
            parameters.max_depth = atoi(token);
        } else if (!strcmp(token, "nodes")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                break;
            }
            parameters.max_nodes = atoi(token);
        } else if (!strcmp(token, "mate")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                break;
            }
            parameters.mate = atoi(token);
        } else if (!strcmp(token, "movetime")) {
            if (!(token = strtok_r(input, " \t", &input))) {
                break;
            }
            parameters.move_time = atoi(token);
        } else if (!strcmp(token, "infinite")) {
            parameters.infinite = true;
        }
    }

    // Set infinite search if no time controls are set
    if (parameters.white_time == 0 && parameters.black_time == 0 &&
        parameters.move_time) {
        parameters.infinite = true;
    }

    if (search_tid) {
        old_tid = search_tid;
        search_tid = 0;
    }

    argument.board = board;
    argument.parameters = parameters;
    pthread_create(&search_tid, NULL, search_thread, &argument);
}

// Parse move from UCI command and return move
static inline Move parse_move(char *move, Board *board) {
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

// Initialize all data structures
static void *init_all(void *board) {
    init_attacks();
    init_board(board);
    init_evaluation();
    init_transposition(512);
    load_fen(board, START_FEN);

    return NULL;
}

// Start searching
static void *search_thread() {
    Board *board = argument.board;
    Parameter parameters = argument.parameters;

    if (old_tid) {
        pthread_join(old_tid, NULL);
    }

    start_search(board, parameters);

    Node *node;
    bool next_search = false;
    while ((node = dequeue())) {
        if (node->function == parse_go) {
            next_search = true;
        }

        node->function(node->input, board);
        free(node->input);
        free(node);

        if (next_search) {
            break;
        }
    }

    if (!next_search) {
        idle = true;
    }

    return NULL;
}

// Add a new function to the end of the queue
static inline void enqueue(void (*function)(char *, Board *), char *input) {
    Node *node = malloc(sizeof(Node));
    node->function = function;
    node->input = strdup(input);
    node->next = NULL;

    if (queue.tail) {
        queue.tail->next = node;
    }
    queue.tail = node;

    if (!queue.head) {
        queue.head = node;
    }
}

// Remove and return the first function in the queue
static inline Node *dequeue() {
    if (!queue.head) {
        return NULL;
    }

    Node *first = queue.head;
    queue.head = queue.head->next;

    if (!queue.head) {
        queue.tail = NULL;
    }

    return first;
}
