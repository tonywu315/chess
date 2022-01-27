#include "transposition.h"

static U64 piece_square_key[12][64];
static U64 castling_key[16];
static U64 enpassant_key[64 + 1];
static U64 side_key[2];

Transposition *transposition = NULL;
int transposition_size = 0;

void init_hash_keys() {
    for (int square = A1; square <= H8; square++) {
        for (int piece = PAWN; piece <= KING + 6; piece++) {
            piece_square_key[piece][square] = rand64();
        }
        enpassant_key[square] = rand64();
    }
    enpassant_key[NO_SQUARE] = 0;

    for (int castling = 0; castling < 16; castling++) {
        castling_key[castling] = rand64();
    }

    side_key[0] = 0;
    side_key[1] = rand64();
}

void init_transposition_size(int megabytes) {
    if (megabytes & (megabytes - 1) != 0) {
        fprintf(stderr, "Error: transposition table size must be power of 2");
    }

    transposition_size = megabytes * 0x100000 / sizeof(Transposition);

    if (transposition != NULL) {
        free(transposition);
    }

    transposition = malloc(transposition_size * sizeof(Transposition));
    if (transposition == NULL) {
        fprintf(stderr, "Error: transposition table failed to allocate");
        exit(1);
    }
}

void clear_transposition() {
    memset(transposition, 0, transposition_size * sizeof(Transposition));
}

int get_transposition(U64 hash, int depth, int alpha, int beta, Move *move) {
    Transposition *entry = &transposition[hash & (transposition_size - 1)];

    if (entry->hash == hash) {
        *move = entry->move;

        if (entry->depth >= depth) {
            if (entry->flag == TRANSPOSITION_EXACT) {
                return entry->score;
            }
            if (entry->flag == TRANSPOSITION_ALPHA && entry->score <= alpha) {
                return alpha;
            }
            if (entry->flag == TRANSPOSITION_BETA && entry->score >= beta) {
                return beta;
            }
        }
    }

    return INVALID_SCORE;
}

void set_transposition(U64 hash, int depth, int score, int flag, Move move) {
    Transposition *entry = &transposition[hash & (transposition_size - 1)];

    if (DEBUG_FLAG && entry->hash == hash) {
        debug_printf("Hash collision\n");
    }

    if (entry->age + 10 < game_ply || entry->depth < depth) {
        entry->hash = hash;
        entry->move = move;
        entry->score = score;
        entry->age = game_ply;
        entry->depth = depth;
        entry->flag = flag;
    }

}

U64 get_hash(Board *board) {
    U64 hash_key = UINT64_C(0), pieces;

    for (int piece = PAWN; piece <= KING + 6; piece++) {
        pieces = board->pieces[piece];
        while (pieces) {
            hash_key ^= piece_square_key[piece][pop_lsb(&pieces)];
        }
    }

    hash_key ^= castling_key[board->state[0].castling];
    hash_key ^= enpassant_key[board->state[0].enpassant];
    hash_key ^= side_key[board->player];

    return hash_key;
}
