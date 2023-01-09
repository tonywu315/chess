#include "transposition.h"
#include "move.h"

U64 piece_key[16][64];
U64 castling_key[16];
U64 enpassant_key[64 + 1];
U64 side_key;

static Transposition *transposition = NULL;
static U64 transposition_size;

static void init_hash_keys();
static void print_pv_moves(Board *board);

// Initialize transposition table
void init_transposition(int megabytes) {
    init_hash_keys();

    // Round megabytes down to previous power of 2
    if (megabytes <= 0) {
        megabytes = 1;
    } else if ((megabytes & (megabytes - 1)) != 0) {
        megabytes |= megabytes >> 1;
        megabytes |= megabytes >> 2;
        megabytes |= megabytes >> 4;
        megabytes |= megabytes >> 8;
        megabytes |= megabytes >> 16;
        megabytes -= megabytes >> 1;
    }

    transposition_size =
        (U64)megabytes * UINT64_C(0x100000) / sizeof(Transposition);

    // Free memory if it is already allocated
    if (transposition != NULL) {
        free(transposition);
    }

    // Dynamically allocate transposition array
    transposition = calloc(transposition_size, sizeof(Transposition));
    if (transposition == NULL) {
        fprintf(stderr, "Error: transposition table failed to allocate\n");
        exit(1);
    }
}

// Clear transposition table
void clear_transposition() {
    memset(transposition, 0, transposition_size * sizeof(Transposition));
}

// Free dynamically allocated memory
void free_transposition() {
    if (transposition) {
        free(transposition);
    }
}

// Check transposition table to see if position has already been searched
int get_transposition(U64 hash, int alpha, int beta, int ply, int depth,
                      Move *move) {
    Transposition *entry = &transposition[hash & (transposition_size - 1)];

    if (entry->hash == hash) {
        *move = entry->move;

        // Only retrieve entries with sufficient depth
        if (entry->depth >= depth) {
            int score = entry->score;

            // Adjust mate score based off of the root node
            if (is_mate_score(score)) {
                score += (score > 0) ? -ply : ply;
            }

            if (entry->flag == EXACT_BOUND) {
                return score;
            }
            if (entry->flag == UPPER_BOUND && score <= alpha) {
                return alpha;
            }
            if (entry->flag == LOWER_BOUND && score >= beta) {
                return beta;
            }
        }
    }

    return INVALID_SCORE;
}

// Save position and score to transposition table
void set_transposition(U64 hash, int score, int flag, int ply, int depth,
                       Move move) {
    Transposition *entry = &transposition[hash & (transposition_size - 1)];

    // Adjust mate score based off of the root node
    if (is_mate_score(score)) {
        score += (score > 0) ? ply : -ply;
    }

    // Replace entry if entry is different or lower depth
    if (entry->hash != hash || entry->depth <= depth) {
        entry->hash = hash;
        entry->move = move;
        entry->score = score;
        // entry->age = game_ply;
        entry->depth = depth;
        entry->flag = flag;
    }
}

// Save principal variation moves to transposition table
void set_pv_moves(Board *board, Stack *stack, int score) {
    Transposition *entry;
    Move *pv_moves = stack->pv_moves;
    int length = stack->pv_length;

    for (int i = 0; i < length; i++) {
        entry = &transposition[board->hash & (transposition_size - 1)];

        // Adjust mate score based off of the root node
        if (is_mate_score(score)) {
            score += (score > 0) ? i : -i;
        }

        entry->hash = board->hash;
        entry->move = pv_moves[i];
        entry->score = (i & 1) ? -score : score;
        // entry->age = game_ply;
        entry->depth = length - i;
        entry->flag = EXACT_BOUND;

        make_move(board, pv_moves[i]);
    }

    for (int i = length - 1; i >= 0; i--) {
        unmake_move(board, pv_moves[i]);
    }
}

// Retrieve principal variation from transposition table
void get_pv_moves(Board *board) {
    printf("\nBest moves:");
    print_pv_moves(board);
    printf("\n");
}

// Get zobrist hash of current position iteratively for debug purposes
U64 get_hash(Board *board) {
    U64 hash_key = UINT64_C(0);
    Bitboard white_pieces, black_pieces;

    for (int piece = PAWN; piece <= KING; piece++) {
        white_pieces = board->pieces[piece];
        black_pieces = board->pieces[piece + 8];
        while (white_pieces) {
            hash_key ^= piece_key[piece][pop_lsb(&white_pieces)];
        }
        while (black_pieces) {
            hash_key ^= piece_key[piece + 8][pop_lsb(&black_pieces)];
        }
    }

    hash_key ^= castling_key[board->state[board->ply].castling];
    hash_key ^= enpassant_key[board->state[board->ply].enpassant];
    hash_key ^= board->player == BLACK ? side_key : 0;

    return hash_key;
}

// Generate random hash keys for transposition table
static void init_hash_keys() {
    for (int square = A1; square <= H8; square++) {
        for (int piece = PAWN; piece <= KING; piece++) {
            piece_key[piece][square] = rand64();
            piece_key[piece + 8][square] = rand64();
        }
        enpassant_key[square] = rand64();
    }
    enpassant_key[NO_SQUARE] = 0;

    for (int castling = 0; castling < 16; castling++) {
        castling_key[castling] = rand64();
    }

    side_key = rand64();
}

// Display the principal variation from tranposition table
static void print_pv_moves(Board *board) {
    U64 hash = board->hash;
    Transposition *entry = &transposition[hash & (transposition_size - 1)];

    if (entry->hash == hash && entry->flag == EXACT_BOUND) {
        Move move = entry->move;
        printf(" %s%s", get_coordinates(get_move_start(move)),
               get_coordinates(get_move_end(move)));

        if (entry->depth != 1) {
            make_move(board, move);
            print_pv_moves(board);
            unmake_move(board, move);
        }
    }
}

// Approximate how full the transposition table is in permill
int get_hashfull() {
    int count = 0;

    for (int i = 0; i < 65536; i++) {
        if (transposition[i].hash) {
            count++;
        }
    }

    return (count * 1000) >> 16;
}
