#include "board.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"

static void start_game(bool mode, int player, int depth);
static int get_move(Move *move, bool mode);
static int game_status();

/* Start single player game */
void start_singleplayer(int player_color, int depth) {
    start_game(true, player_color, depth);
}

/* Start multi player game */
void start_multiplayer() {
    start_game(false, 0, 0);
}

/* Start chess game */
static void start_game(bool mode, int player, int depth) {
    int status, score = eval();
    bool flag = true;

    printf("\n=== Chess Program ===\n");

    /* Computer moves first if player is black */
    if (mode && player == BLACK) {
        score = -move_computer(depth);
        if ((status = game_status())) {
            flag = false;
        }
    }

    while (flag) {
        /* Print board */
        print_board(score);

        /* Get move until it is legal */
        Move move;
        while (get_move(&move, mode) || move_legal(&move)) {
        }

        /* Stop game if it is over */
        if ((status = game_status())) {
            score = INT_MAX;
            board.player = 3 - board.player;
            break;
        }
        score = eval();

        /* Play computer move if in single player mode */
        if (mode) {
            score = -move_computer(depth);
            if ((status = game_status())) {
                score = INT_MAX;
                break;
            }
        }
    }

    print_board(score);
    if (status == WHITE_WIN) {
        printf("\nCheckmate! White wins\n");
    } else if (status == BLACK_WIN) {
        printf("\nCheckmate! Black wins\n");
    } else {
        printf("\nStalemate\n");
    }
}

/* Parses player move and returns move */
static int get_move(Move *move, bool mode) {
    char input[5] = {0};
    int start_file, start_rank, end_file, end_rank, promotion = 0;

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
        promotion = PROMOTION_N;
        break;
    case 'b':
        promotion = PROMOTION_B;
        break;
    case 'r':
        promotion = PROMOTION_R;
        break;
    case 'q':
        promotion = PROMOTION_Q;
        break;
    }

    /* Option to undo moves */
    if (!strcmp(input, "undo")) {
        /* If single player, undo 2 moves. Otherwise, undo 1 move */
        if (mode) {
            if (game_position >= 2) {
                unmove_piece();
                unmove_piece();
            }
        } else if (game_position >= 1) {
            unmove_piece();
        }

        print_board(eval());
        return FAILURE;
    }

    /* Kingside castling */
    if (!strcmp(input, "O-O") || !strcmp(input, "0-0")) {
        if (board.player == WHITE) {
            create_move(move, E1, G1, CASTLE_WK);
        } else {
            create_move(move, E8, G8, CASTLE_BK);
        }

        return SUCCESS;
    }

    /* Queenside castling */
    if (!strcmp(input, "O-O-O") || !strcmp(input, "0-0-0")) {
        if (board.player == WHITE) {
            create_move(move, E1, C1, CASTLE_WQ);
        } else {
            create_move(move, E8, C8, CASTLE_BQ);
        }

        return SUCCESS;
    }

    /* Create normal move */
    if (valid_row(start_file) && valid_row(start_rank) && valid_row(end_file) &&
        valid_row(end_rank)) {
        U8 start = get_square(start_file, start_rank);
        U8 end = get_square(end_file, end_rank);

        /* Promotion */
        if (board.pieces[start] == PAWN &&
            start_rank == (board.player == WHITE ? 6 : 1)) {
            if (promotion == 0) {
                return FAILURE;
            }
            create_move(move, start, end, promotion);
        } else {
            create_move(move, start, end, NORMAL);
        }

        return SUCCESS;
    }

    return FAILURE;
}

/* Returns status of game (white win, black win, draw, or nothing) */
static int game_status() {
    Move moves[MAX_MOVES];

    if (generate_legal_moves(moves) == 0) {
        if (in_check(board.player)) {
            return board.player == WHITE ? BLACK_WIN : WHITE_WIN;
        }
        return DRAW;
    }

    return NORMAL;
}
