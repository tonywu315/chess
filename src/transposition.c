#include "transposition.h"

U64 piece_key[16][64];
U64 castling_key[16];
U64 enpassant_key[64 + 1];
U64 side_key;

Transposition *transposition = NULL;
int transposition_size = 0;

static inline void init_hash_keys();

// Initialize transposition table
void init_transposition(int megabytes) {
    init_hash_keys();

    // Check that size is a power of 2
    if ((megabytes & (megabytes - 1)) != 0) {
        fprintf(stderr, "Error: transposition table size must be power of 2\n");
        exit(1);
    }

    transposition_size = megabytes * 0x100000 / sizeof(Transposition);

    // Free memory if it is already allocated, shouldn't happen
    if (transposition != NULL) {
        fprintf(stderr, "Error: init_transposition called twice\n");
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

// Check transposition table to see if position has already been searched
int get_transposition(U64 hash, int alpha, int beta, int ply, int depth,
                      Move *move) {
    Transposition *entry = &transposition[hash & (transposition_size - 1)];

    if (entry->hash == hash) {
        *move = entry->move;

        if (entry->depth >= depth) {
            // Adjust mate score based off of the root node
            int score = entry->score;
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
        entry->age = game_ply;
        entry->depth = depth;
        entry->flag = flag;
    }
}

// Get zobrist hash of current position, this function is for debug purposes
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
static inline void init_hash_keys() {
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
