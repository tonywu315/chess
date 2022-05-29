#include "game.h"
#include "attacks.h"
#include "bitboard.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "search.h"

static Move game_moves[MAX_GAME_LENGTH];
int game_ply = 0;

static void start_game(Board *board, bool single, bool player_first, int time);
static int get_move(Board *board, Move *move, bool single);
static int move_legal(Board *board, Move move);
static int game_status(Board *board);

// Start singleplayer game
void start_singleplayer(Board *board, bool player_starts, int time) {
    start_game(board, true, player_starts, time);
}

// Start multiplayer game
void start_multiplayer(Board *board) { start_game(board, false, true, 0); }

// Start chess game
static void start_game(Board *board, bool single, bool player_first, int time) {
    Move move;
    int score = eval(board);

    printf("\n=== Chess v%s ===\n", VERSION);

    // Computer moves first if player is black
    if (single && !player_first) {
        score = search_position(board, &move, time);
        make_move(board, move);
        game_moves[game_ply++] = move;
    }

    // Loop until game ends
    int status = game_status(board);
    while (status == NO_STATUS) {
        // Print board
        print_board(board, score, false);

        // Input move until it is legal and then make the move
        while (get_move(board, &move, single) || move_legal(board, move)) {
        }
        game_moves[game_ply++] = move;

        // Stop game once it ends
        if ((status = game_status(board))) {
            board->player = !board->player;
            break;
        }
        score = eval(board);

        // Computer moves if it is singleplayer
        if (single) {
            score = search_position(board, &move, time);
            make_move(board, move);
            game_moves[game_ply++] = move;
            status = game_status(board);
        }
    }

    // Print the board at the end of the game
    print_board(board, score, true);
    if (status == WHITE_WIN) {
        printf("Checkmate! White wins\n");
    } else if (status == BLACK_WIN) {
        printf("Checkmate! Black wins\n");
    } else {
        printf("Stalemate\n");
    }
}

// Parse player move and returns move
static int get_move(Board *board, Move *move, bool single) {
    char input[6] = {0};
    int start_file, start_rank, end_file, end_rank, promotion = NO_PIECE_TYPE;

    printf("Move: ");
    if (!scanf("%5s", input)) {
        return FAILURE;
    }

    // Get files and ranks in integer representation
    start_file = tolower(input[0]) - 'a';
    start_rank = input[1] - '1';
    end_file = tolower(input[2]) - 'a';
    end_rank = input[3] - '1';

    // Get promotion piece
    switch (tolower(input[4])) {
    case 'n':
        promotion = KNIGHT;
        break;
    case 'b':
        promotion = BISHOP;
        break;
    case 'r':
        promotion = ROOK;
        break;
    case 'q':
        promotion = QUEEN;
        break;
    }

    // Option to undo moves
    if (!strcmp(input, "undo")) {
        // If single player, undo 2 moves. Otherwise, undo 1 move
        if (single) {
            if (game_ply >= 2) {
                unmake_move(board, game_moves[--game_ply]);
                unmake_move(board, game_moves[--game_ply]);
            }
        } else if (game_ply >= 1) {
            unmake_move(board, game_moves[--game_ply]);
        }

        print_board(board, eval(board), false);
        return FAILURE;
    }

    // Kingside castling
    if (!strcmp(input, "O-O") || !strcmp(input, "0-0")) {
        if (board->player == WHITE) {
            *move = UINT16_C(0xF1C4);
        } else {
            *move = UINT16_C(0xFFFC);
        }

        return SUCCESS;
    }

    // Queenside castling
    if (!strcmp(input, "O-O-O") || !strcmp(input, "0-0-0")) {
        if (board->player == WHITE) {
            *move = UINT16_C(0xF004);
        } else {
            *move = UINT16_C(0xFE3C);
        }

        return SUCCESS;
    }

    // Create normal move
    if (valid_row(start_file) && valid_row(start_rank) && valid_row(end_file) &&
        valid_row(end_rank)) {
        int start = make_square(start_file, start_rank);
        int end = make_square(end_file, end_rank);

        // Promotion
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

// Make move if it is legal
static int move_legal(Board *board, Move move) {
    Move moves[MAX_MOVES];
    int count = generate_legal_moves(board, moves);

    // Iterate through all legal moves and check if the move is in there */
    for (int i = 0; i < count; i++) {
        if ((moves[i] & UINT16_C(0xCFFF)) == (move & UINT16_C(0xCFFF))) {
            make_move(board, moves[i]);
            return SUCCESS;
        }
    }

    return FAILURE;
}

// Returns status of game (white win, black win, draw, or nothing)
static int game_status(Board *board) {
    Move moves[MAX_MOVES];

    if (generate_legal_moves(board, moves) == 0) {
        if (in_check(board, board->player)) {
            return board->player == WHITE ? BLACK_WIN : WHITE_WIN;
        }
        return DRAW;
    }

    return NO_STATUS;
}
