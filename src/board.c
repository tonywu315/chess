#include "board.h"
#include "move.h"
#include "move_generation.h"

static void san_to_move(Move *move, const char *san, U8 player);
static void create_pawn_move(Move *move, const char *san, U8 player, int len);

/* Returns index in 0x88 board */
static inline int get_index(char file, char rank) {
    return 16 * (rank - '1') + file - 'a';
}

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
    board.king[BLACK - 1] = 255;

    game_position = 0;
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

    /* Print score unless end of game */
    if (score != INT_MAX) {
        /* Prints mate in x if detected */
        if (abs(score) >= INT_MAX - 100) {
            printf("\nMate in %d\n", (INT_MAX - abs(score)) / 2);
        } else {
            printf("\nEvaluation: %d\n", score);
        }

        printf("Player to move: %s\n", players[(int)board.player]);
    }
}

/* Loads a board from FEN representation */
void load_fen(const char *fen) {
    int i = 0, j = A8;

    init_board(board);

    /* Adds piece depending on character */
    do {
        switch (fen[i]) {
        case 'P':
            board.colors[j] = WHITE;
            board.pieces[j++] = PAWN;
            break;
        case 'N':
            board.colors[j] = WHITE;
            board.pieces[j++] = KNIGHT;
            break;
        case 'B':
            board.colors[j] = WHITE;
            board.pieces[j++] = BISHOP;
            break;
        case 'R':
            board.colors[j] = WHITE;
            board.pieces[j++] = ROOK;
            break;
        case 'Q':
            board.colors[j] = WHITE;
            board.pieces[j++] = QUEEN;
            break;
        case 'K':
            board.king[WHITE - 1] = j;
            board.colors[j] = WHITE;
            board.pieces[j++] = KING;
            break;
        case 'p':
            board.colors[j] = BLACK;
            board.pieces[j++] = PAWN;
            break;
        case 'n':
            board.colors[j] = BLACK;
            board.pieces[j++] = KNIGHT;
            break;
        case 'b':
            board.colors[j] = BLACK;
            board.pieces[j++] = BISHOP;
            break;
        case 'r':
            board.colors[j] = BLACK;
            board.pieces[j++] = ROOK;
            break;
        case 'q':
            board.colors[j] = BLACK;
            board.pieces[j++] = QUEEN;
            break;
        case 'k':
            board.king[BLACK - 1] = j;
            board.colors[j] = BLACK;
            board.pieces[j++] = KING;
            break;
        case '/':
            j -= 24;
            break;
        default:
            j += fen[i] - '0';
        }
    } while (fen[++i] != ' ');

    /* Side to move */
    if (fen[++i] == 'w') {
        board.player = WHITE;
    } else {
        board.player = BLACK;
    }

    i += 2;

    /* Castle rights */
    board.castle = 0;
    do {
        switch (fen[i]) {
        case 'K':
            board.castle |= CASTLE_WK;
            break;
        case 'Q':
            board.castle |= CASTLE_WQ;
            break;
        case 'k':
            board.castle |= CASTLE_BK;
            break;
        case 'q':
            board.castle |= CASTLE_BQ;
            break;
        }
    } while (fen[++i] != ' ');

    /* Enpassant square */
    if (fen[++i] == '-') {
        i += 2;
    } else {
        board.enpassant = fen[i] - 'a' + 16 * (fen[i + 1] - '1');
        i += 3;
    }

    board.ply = fen[i] - '0';
}

/* Loads a game from PGN representation */
void load_pgn(const char *pgn) {
    Move move;
    int size = strlen(pgn), j = 0, move_count = 1;
    char san[128];

    start_board();

    for (int i = 0; i < size; i++) {
        while (pgn[i] != ' ') {
            i++;
        }

        /* Alternate white and black moves */
        for (U8 player = WHITE; player <= BLACK; player++) {
            while (pgn[++i] != ' ' && pgn[i] != '\0') {
                san[j++] = pgn[i];
                san[j] = '\0';
            }
            j = 0;

            /* Resets board if move is illegal */
            san_to_move(&move, san, player);
            if (move_legal(&move)) {
                printf("Error: could not load PGN\n");
                printf("Invalid move %d: %c%c %c%c %d (%s)\n", move_count,
                       get_file(move.start) + 'A', get_rank(move.start) + '1',
                       get_file(move.end) + 'A', get_rank(move.end) + '1',
                       move.flag, san);
                print_board(0);
                start_board();
                return;
            }
            move_count++;

            /* Stops game at the end */
            if (pgn[i] == '\0') {
                return;
            }
        }

        while (pgn[i] != ' ') {
            i++;
        }
    }
}

/* Converts standard algebraic notation to move */
static void san_to_move(Move *move, const char *san, U8 player) {
    int length = strlen(san);

    /* Removes extra symbols */
    while (san[length - 1] == '+' || san[length - 1] == '#' ||
           san[length - 1] == '!' || san[length - 1] == '?') {
        length--;
    }

    /* Castling moves */
    if (player == WHITE) {
        if (length == 3 && !strncmp(san, "O-O", 3)) {
            create_move(move, E1, G1, CASTLE_WK);
            return;
        } else if (length == 5 && !strncmp(san, "O-O-O", 5)) {
            create_move(move, E1, A1, CASTLE_WQ);
            return;
        }
    } else {
        if (length == 3 && !strncmp(san, "O-O", 3)) {
            create_move(move, E8, G8, CASTLE_BK);
            return;
        } else if (length == 5 && !strncmp(san, "O-O-O", 5)) {
            create_move(move, E8, A8, CASTLE_BQ);
            return;
        }
    }

    /* Pawn move */
    if (islower(san[0])) {
        create_pawn_move(move, san, player, length);
    } else {
        U8 piece, end = get_index(san[length - 2], san[length - 1]), valid[8];
        Move moves[MAX_MOVES];
        int j = 0, count = generate_legal_moves(moves), flag = NORMAL;

        switch (san[0]) {
        case 'N':
        case 'n':
            piece = KNIGHT;
            break;
        case 'B':
        case 'b':
            piece = BISHOP;
            break;
        case 'R':
        case 'r':
            piece = ROOK;
            break;
        case 'Q':
        case 'q':
            piece = QUEEN;
            break;
        case 'K':
        case 'k':
            piece = KING;
            break;
        }

        /* Capture */
        if (san[length - 3] == 'x') {
            flag = CAPTURE;
            length--;
        }

        /* Array of valid moves to the square */
        for (int i = 0; i < count; i++) {
            if (board.colors[moves[i].start] == player &&
                board.pieces[moves[i].start] == piece && moves[i].end == end) {
                valid[j++] = moves[i].start;
            }
        }

        /* Create moves */
        if (length == 3) {
            create_move(move, valid[0], end, flag);
        } else if (length == 4) {
            /* Resolves ambiguity by searching valid moves to square */
            if (san[1] >= 'a' && san[1] <= 'h') {
                int start = -1;
                for (int i = 0; i < j; i++) {
                    if (get_file(valid[i]) == san[1] - 'a') {
                        /* Do not create move if ambiguous */
                        if (start != -1) {
                            return;
                        }
                        start = valid[i];
                    }
                }
                create_move(move, start, end, flag);
            } else {
                int start = -1;
                for (int i = 0; i < j; i++) {
                    if (get_rank(valid[i]) == san[1] - '1') {
                        /* Do not create move if ambiguous */
                        if (start != -1) {
                            return;
                        }
                        start = valid[i];
                    }
                }
                create_move(move, start, end, flag);
            }
        } else if (length == 5) {
            /* Starting square is explicit */
            create_move(move, get_index(san[1], san[2]), end, flag);
        }
    }
}

/* Creates pawn move */
static void create_pawn_move(Move *move, const char *san, U8 player, int len) {
    int flag, up;
    char file, rank, mid, end;

    if (player == WHITE) {
        up = 1;
        mid = '4';
        end = '8';
    } else {
        up = -1;
        mid = '5';
        end = '1';
    }
    if (san[1] == 'x') {
        flag = CAPTURE;
        file = san[2];
        rank = san[3];
    } else {
        flag = NORMAL;
        file = san[0];
        rank = san[1];
    }

    /* Promotion */
    if (rank == end) {
        switch (san[len - 1]) {
        case 'N':
            create_move(move, get_index(san[0], end - up), get_index(file, end),
                        PROMOTION_N);
            break;
        case 'B':
            create_move(move, get_index(san[0], end - up), get_index(file, end),
                        PROMOTION_B);
            break;
        case 'R':
            create_move(move, get_index(san[0], end - up), get_index(file, end),
                        PROMOTION_R);
            break;
        case 'Q':
            create_move(move, get_index(san[0], end - up), get_index(file, end),
                        PROMOTION_Q);
            break;
        }
    } else if (flag) {
        /* Capture */
        create_move(move, get_index(san[0], rank - up), get_index(file, rank),
                    flag);
    } else {
        /* Double pawn move */
        int start = get_index(file, mid - up - up);
        int second = get_index(file, mid - up);
        if (rank == mid && !board.colors[second] &&
            board.colors[start] == player && board.pieces[start] == PAWN) {
            create_move(move, start, get_index(file, rank), ENPASSANT);
        } else {
            /* Single pawn move */
            start = get_index(san[0], rank - up);
            if (board.colors[start] == player && board.pieces[start] == PAWN) {
                create_move(move, start, get_index(file, rank), flag);
            }
        }
    }
}
