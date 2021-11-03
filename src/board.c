#include "board.h"
#include "move_generation.h"

/* Returns true if square is outside the board */
inline int invalid_square(U8 square) { return square & 0x88; }

/* Returns rank and file of square (number from 0 to 7) */
inline int get_rank(U8 square) { return square >> 4; }
inline int get_file(U8 square) { return square & 7; }

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

/* Creates pawn move */
static inline void create_pawn_move(Move *move, const char *san, U8 player) {
    int flag, up;
    char rank, mid, end;

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
        rank = san[2];
    } else {
        flag = NORMAL;
        rank = san[0];
    }

    /* Promotion */
    if ((flag && san[3] == end) || (!flag && san[1] == end)) {
        switch (san[rank + 3]) {
        case 'N':
            create_move(move, get_index(san[0], end - up), get_index(rank, end),
                        PROMOTION_N);
            break;
        case 'B':
            create_move(move, get_index(san[0], end - up), get_index(rank, end),
                        PROMOTION_B);
            break;
        case 'R':
            create_move(move, get_index(san[0], end - up), get_index(rank, end),
                        PROMOTION_R);
            break;
        case 'Q':
            create_move(move, get_index(san[0], end - up), get_index(rank, end),
                        PROMOTION_Q);
            break;
        }
    } else if (flag) {
        /* Capture */
        create_move(move, get_index(san[0], san[3] - up),
                    get_index(rank, san[3]), flag);
    } else if (san[1] == mid) {
        /* Double pawn move */
        int start = get_index(san[0], mid - up - up);
        int next = get_index(san[0], mid - up);
        if (!board.colors[next] && board.colors[start] == player &&
            board.pieces[start] == PAWN) {
            create_move(move, start, get_index(rank, san[1]), ENPASSANT);
        }
    } else {
        /* Single pawn move */
        int start = get_index(san[0], mid - up - up);
        if (board.colors[start] == player && board.pieces[start] == PAWN) {
            create_move(move, start, get_index(rank, san[1]), flag);
        }
    }
}

/* TODO: use regex to validate SAN */
/* Converts standard algebraic notation to move */
void san_to_move(Move *move, const char *san, U8 player) {
    int length = strlen(san);

    if (san[length - 1] == '+' || san[length - 1] == '#') {
        length--;
    }

    /* Castling moves */
    if (player == WHITE) {
        if (length >= 3 && !strncmp(san, "O-O", 3)) {
            create_move(move, E1, G1, CASTLE_WK);
            return;
        } else if (length >= 5 && !strncmp(san, "O-O-O", 5)) {
            create_move(move, E1, A1, CASTLE_WQ);
            return;
        }
    } else {
        if (length >= 3 && !strncmp(san, "O-O", 3)) {
            create_move(move, E8, G8, CASTLE_BK);
            return;
        } else if (length >= 5 && !strncmp(san, "O-O-O", 5)) {
            create_move(move, E8, A8, CASTLE_BQ);
            return;
        }
    }

    /* Pawn move */
    if (islower(san[0])) {
        create_pawn_move(move, san, player);
    } else {
        U8 piece, end = get_index(san[length - 2], san[length - 1]);
        Move moves[MAX_MOVES];
        int i = 0, count = generate_legal_moves(moves);

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

        if (length == 3) {
            for (i = 0; i < count; i++) {
                if (board.colors[moves[i].start] == player &&
                    board.pieces[moves[i].start] == piece &&
                    moves[i].end == end) {
                    create_move(move, moves[i].start, end, NORMAL);
                }
            }
        } else if (length == 4) {
            if (san[1] == 'x') {
                for (i = 0; i < count; i++) {
                    if (board.colors[moves[i].start] == player &&
                        board.pieces[moves[i].start] == piece &&
                        moves[i].end == end) {
                        create_move(move, moves[i].start, end, CAPTURE);
                    }
                }
            } else {
                if (san[1] >= 'a' && san[1] <= 'h') {
                    for (i = 0; i < count; i++) {
                        if (board.colors[moves[i].start] == player &&
                            board.pieces[moves[i].start] == piece &&
                            get_file(moves[i].start) == san[1] &&
                            moves[i].end == end) {
                            create_move(move, moves[i].start, end, NORMAL);
                        }
                    }
                } else {
                    for (i = 0; i < count; i++) {
                        if (board.colors[moves[i].start] == player &&
                            board.pieces[moves[i].start] == piece &&
                            get_rank(moves[i].start) == san[1] &&
                            moves[i].end == end) {
                            create_move(move, moves[i].start, end, NORMAL);
                        }
                    }
                }
            }
        } else if (length == 5) {
            if (san[2] == 'x') {
                if (san[1] >= 'a' && san[1] <= 'h') {
                    for (i = 0; i < count; i++) {
                        if (board.colors[moves[i].start] == player &&
                            board.pieces[moves[i].start] == piece &&
                            get_file(moves[i].start) == san[1] &&
                            moves[i].end == end) {
                            create_move(move, moves[i].start, end, CAPTURE);
                        }
                    }
                } else {
                    for (i = 0; i < count; i++) {
                        if (board.colors[moves[i].start] == player &&
                            board.pieces[moves[i].start] == piece &&
                            get_rank(moves[i].start) == san[1] &&
                            moves[i].end == end) {
                            create_move(move, moves[i].start, end, CAPTURE);
                        }
                    }
                }
            } else {
                create_move(move, get_index(san[1], san[2]), end, NORMAL);
            }
        } else {
            create_move(move, get_index(san[1], san[2]), end, NORMAL);
        }
    }
}

/* Converts move to standard algebraic notation
void move_to_san(char *san, const Move *move, U8 player) {}
*/

/* Loads a game from PGN representation
void load_pgn(const char *fen) {}
*/

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