#include "bitboard.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "attacks.h"

static void start_game(Board *board, bool mode, int player, int depth);
static int get_move(const Board *board, Move *move);
static int game_status(Board *board);

/* Start single player game */
void start_singleplayer(Board *board, int player_color, int depth) {
    start_game(board, true, player_color, depth);
}

/* Start multi player game */
void start_multiplayer(Board *board) {
    start_game(board, false, 0, 0);
}

/* Start chess game */
static void start_game(Board *board, bool mode, int player, int depth) {
    int status = 0, score = eval(board);
    bool flag = true;

    printf("\n=== Chess Program ===\n");

    /* Computer moves first if player is black */
    if (mode && player == BLACK) {
        score = -move_computer(board, depth);
        if ((status = game_status(board))) {
            flag = false;
        }
    }

    while (flag) {
        /* Print board */
        print_board(board, score);

        /* Get move until it is legal */
        Move move;
        while (get_move(board, &move) || move_legal(board, move)) {
        }

        /* Stop game if it is over */
        if ((status = game_status(board))) {
            score = INT_MAX;
            board->player = !board->player;
            break;
        }
        score = eval(board);

        /* Play computer move if in single player mode */
        if (mode) {
            score = -move_computer(board, depth);
            if ((status = game_status(board))) {
                score = INT_MAX;
                break;
            }
        }
    }

    print_board(board, score);
    if (status == WHITE_WIN) {
        printf("\nCheckmate! White wins\n");
    } else if (status == BLACK_WIN) {
        printf("\nCheckmate! Black wins\n");
    } else {
        printf("\nStalemate\n");
    }
}

/* Parses player move and returns move */
static int get_move(const Board *board, Move *move) {
    char input[5] = {0};
    int start_file, start_rank, end_file, end_rank, promotion = NO_PIECE_TYPE;

    printf("Move: ");
    if (!scanf("%5s", input)) {
        return FAILURE;
    }

    /* Returns integer equivalent of chars */
    start_file = tolower(input[0]) - 'a';
    start_rank = input[1] - '1';
    end_file = tolower(input[2]) - 'a';
    end_rank = input[3] - '1';

    switch (tolower(input[4])) {
    case 'n':
        promotion = 0;
        break;
    case 'b':
        promotion = 1;
        break;
    case 'r':
        promotion = 2;
        break;
    case 'q':
        promotion = 3;
        break;
    }

    /* Option to undo moves */
    // if (!strcmp(input, "undo")) {
    //     /* If single player, undo 2 moves. Otherwise, undo 1 move */
    //     if (mode) {
    //         if (game_position >= 2) {
    //             unmove_piece();
    //             unmove_piece();
    //         }
    //     } else if (game_position >= 1) {
    //         unmove_piece();
    //     }

    //     old_print_board(eval());
    //     return FAILURE;
    // }

    /* Kingside castling */
    if (!strcmp(input, "O-O") || !strcmp(input, "0-0")) {
        if (board->player == WHITE) {
            *move = UINT16_C(0xF1C4);
        } else {
            *move = UINT16_C(0xFFFC);
        }

        return SUCCESS;
    }

    /* Queenside castling */
    if (!strcmp(input, "O-O-O") || !strcmp(input, "0-0-0")) {
        if (board->player == WHITE) {
            *move = UINT16_C(0xF004);
        } else {
            *move = UINT16_C(0xFE3C);
        }

        return SUCCESS;
    }

    /* Create normal move */
    if (valid_row(start_file) && valid_row(start_rank) && valid_row(end_file) &&
        valid_row(end_rank)) {
        int start = make_square(start_file, start_rank);
        int end = make_square(end_file, end_rank);

        /* Promotion */
        if (get_piece(board->board[start]) == PAWN &&
            start_rank == (board->player == WHITE ? 6 : 1)) {
            if (promotion == NO_PIECE_TYPE) {
                return FAILURE;
            }
            *move = encode_move(start, end, PROMOTION, promotion);
        } else {
            *move = encode_move(start, end, 0, 0);
        }

        return SUCCESS;
    }

    return FAILURE;
}

/* Returns status of game (white win, black win, draw, or nothing) */
static int game_status(Board *board) {
    Move moves[MAX_MOVES];

    if (generate_legal_moves(board, moves) == 0) {
        if (in_check(board, board->player)) {
            return board->player == WHITE ? BLACK_WIN : WHITE_WIN;
        }
        return DRAW;
    }

    return NONE;
}
