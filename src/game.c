#include "game.h"
#include "attacks.h"
#include "bitboard.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "search.h"

Game game;

static int computer_move(Board *board, int *score, int time);
static int player_move(Board *board);
static int get_move(Board *board, Move *move);
static int move_legal(Board *board, Move move);
static int update_game_moves(Board *board, Move move);

// Start chess game
void start_game(Board *board, int time, bool player_first) {
    int score = eval(board), status = NO_STATUS;

    printf("\n=== Chess v%s ===\n", VERSION);

    // Computer moves first if player is black
    if (!player_first) {
        status = computer_move(board, &score, time);
    }

    // Loop until game ends
    while (!status) {
        // Player move
        print_board(board, score, false);
        status = player_move(board);
        if (status) {
            board->player = !board->player;
            break;
        }

        // Computer move
        status = computer_move(board, &score, time);
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

// Play computer move
static int computer_move(Board *board, int *score, int time) {
    Move move;

    *score = search_position(board, &move, time);
    make_move(board, move);

    return update_game_moves(board, move);
}

// Play player move
static int player_move(Board *board) {
    Move move;

    if (DEBUG_FLAG && replay.is_replay && game.ply < replay.game_ply) {
        move = replay.ply[game.ply].move;
        make_move(board, move);
        printf("Move: %s%s\n", get_coordinates(get_move_start(move)),
               get_coordinates(get_move_end(move)));

        return update_game_moves(board, move);
    }

    // Input move until it is legal and then make the move
    while (get_move(board, &move) || move_legal(board, move)) {
    }
    replay.ply[game.ply].move = move;

    return update_game_moves(board, move);
}

// Parse player move and returns move
static int get_move(Board *board, Move *move) {
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
        // Undo 2 moves
        if (game.ply >= 2) {
            unmake_move(board, game.moves[--game.ply]);
            unmake_move(board, game.moves[--game.ply]);
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
        if (get_piece_type(board->board[start]) == PAWN &&
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

// Update game move and game ply, and return status of game
static int update_game_moves(Board *board, Move move) {
    Move moves[MAX_MOVES];

    game.moves[game.ply++] = move;

    if (generate_legal_moves(board, moves) == 0) {
        if (in_check(board, board->player)) {
            return board->player == WHITE ? BLACK_WIN : WHITE_WIN;
        }
        return DRAW;
    }

    return NO_STATUS;
}
