#include "attacks.h"

// Parameters for perfect hashing algorithm to index attack lookup tables
typedef struct magics {
    Bitboard *attacks;
    Bitboard mask;
    Bitboard magic;
    int shift;
} Magic;

// Lookup tables and Fancy Magic Bitboards
static Bitboard pawn_attacks[2][64];
static Bitboard knight_attacks[64];
static Bitboard king_attacks[64];
static Magic rook_magics[64];
static Magic bishop_magics[64];
static Bitboard rook_attacks[102400];
static Bitboard bishop_attacks[5248];

static Bitboard init_pawn_attacks(int square, int player);
static Bitboard init_knight_attacks(int square);
static Bitboard init_king_attacks(int square);

static void init_magics(int piece);
static Bitboard get_rook_mask(int square);
static Bitboard get_bishop_mask(int square);
static Bitboard get_slider_attack(int square, Bitboard occupancy, int piece);
static inline Bitboard get_rook_attacks(int square, Bitboard occupancy);
static inline Bitboard get_bishop_attacks(int square, Bitboard occupancy);
static Bitboard sparse_random();

// Initialize attack lookup tables
void init_attacks() {
    for (int square = 0; square < 64; square++) {
        pawn_attacks[WHITE][square] = init_pawn_attacks(square, WHITE);
        pawn_attacks[BLACK][square] = init_pawn_attacks(square, BLACK);
        knight_attacks[square] = init_knight_attacks(square);
        king_attacks[square] = init_king_attacks(square);
    }

    // Find perfect hashing algorithm multiply number for rooks and bishops
    init_magics(ROOK);
    init_magics(BISHOP);
}

// Get attack bitboard by piece on square excluding own color
Bitboard get_attacks(const Board *board, int square, int piece) {
    switch (piece) {
    case PAWN:
        return pawn_attacks[board->player][square] &
               board->occupancies[!board->player];
    case KNIGHT:
        return knight_attacks[square] & ~board->occupancies[board->player];
    case BISHOP:
        return get_bishop_attacks(square, board->occupancies[2]) &
               ~board->occupancies[board->player];
    case ROOK:
        return get_rook_attacks(square, board->occupancies[2]) &
               ~board->occupancies[board->player];
    case QUEEN:
        return (get_bishop_attacks(square, board->occupancies[2]) |
                get_rook_attacks(square, board->occupancies[2])) &
               ~board->occupancies[board->player];
    case KING:
        return king_attacks[square] & ~board->occupancies[board->player];
    }

    return UINT64_C(0);
}

// Test if square is attacked by player
bool is_attacked(const Board *board, int square, int player) {
    int shift = player == WHITE ? 0 : 8;
    return (pawn_attacks[!player][square] & board->pieces[PAWN + shift]) ||
           (knight_attacks[square] & board->pieces[KNIGHT + shift]) ||
           (king_attacks[square] & board->pieces[KING + shift]) ||
           (get_rook_attacks(square, board->occupancies[2]) &
            (board->pieces[ROOK + shift] | board->pieces[QUEEN + shift])) ||
           (get_bishop_attacks(square, board->occupancies[2]) &
            (board->pieces[BISHOP + shift] | board->pieces[QUEEN + shift]));
}

// Test if player is in check
bool in_check(const Board *board, int player) {
    int king = get_lsb(board->pieces[make_piece(KING, player)]);
    return is_attacked(board, king, !player);
}

// Initialize pawn attack lookup table
static Bitboard init_pawn_attacks(int square, int player) {
    Bitboard pawn = create_bit(square);

    if (player == WHITE) {
        return shift_bit(pawn, UPLEFT) | shift_bit(pawn, UPRIGHT);
    } else {
        return shift_bit(pawn, DOWNLEFT) | shift_bit(pawn, DOWNRIGHT);
    }
}

// Initialize knight attack looukup table
static Bitboard init_knight_attacks(int square) {
    static const Bitboard MASK_FILE_AB = UINT64_C(0xFCFCFCFCFCFCFCFC);
    static const Bitboard MASK_FILE_HG = UINT64_C(0x3F3F3F3F3F3F3F3F);
    Bitboard knight = create_bit(square);

    set_bit(&knight, square);

    Bitboard left1 = shift_bit(knight, LEFT);
    Bitboard left2 = (knight >> 2) & MASK_FILE_HG;
    Bitboard right1 = shift_bit(knight, RIGHT);
    Bitboard right2 = (knight << 2) & MASK_FILE_AB;
    Bitboard height1 = left1 | right1;
    Bitboard height2 = left2 | right2;

    return (height1 << 16) | (height1 >> 16) | (height2 << 8) | (height2 >> 8);
}

// Initialize king attack lookup table
static Bitboard init_king_attacks(int square) {
    Bitboard king = create_bit(square);
    Bitboard attacks = shift_bit(king, LEFT) | shift_bit(king, RIGHT);

    king |= attacks;
    attacks |= shift_bit(king, UP) | shift_bit(king, DOWN);

    return attacks;
}

// Generate magics
static void init_magics(int piece) {
    Bitboard occupancy[4096], attacks[4096], bitboard = UINT64_C(0);
    int repeated[4096] = {0}, attacks_index = 0, attempts = 0;

    // Iterate over all squares
    for (int square = A1; square <= H8; square++) {
        Magic magic;

        // Set piece masks and starting index of lookup table
        if (piece == ROOK) {
            magic.attacks = &rook_attacks[attacks_index];
            magic.mask = get_rook_mask(square);
        } else {
            magic.attacks = &bishop_attacks[attacks_index];
            magic.mask = get_bishop_mask(square);
        }

        // Set relevant bits
        magic.shift = 64 - get_population(magic.mask);

        // Generate all subsets of piece mask and corresponding attack bitboard
        int count = 0;
        do {
            occupancy[count] = bitboard;
            attacks[count++] = get_slider_attack(square, bitboard, piece);
            bitboard = (bitboard - magic.mask) & magic.mask;
        } while (bitboard);

        // Iterate until a magic is found
        int i = 0;
        do {
            // Generate magic number candidate
            magic.magic = sparse_random();

            // Discard numbers with less than 6 bits set in the first 8 bits
            if (get_population((magic.magic * magic.mask) >> 56) < 6) {
                continue;
            }

            attempts += 1;
            // Iterate over each possible piece mask
            for (i = 0; i < count; i++) {
                unsigned int index =
                    (int)(((occupancy[i] & magic.mask) * magic.magic) >>
                          magic.shift);

                // Condition is only satisfied if all indices are unique
                if (repeated[index] < attempts) {
                    repeated[index] = attempts;
                    magic.attacks[index] = attacks[i];
                } else if (magic.attacks[index] != attacks[i]) {
                    // Break if magic number does not work
                    break;
                }
            }
        } while (i < count);

        // Increase next starting index by number of indices taken
        attacks_index += 1 << (64 - magic.shift);

        // Put magic in table
        if (piece == ROOK) {
            rook_magics[square] = magic;
        } else {
            bishop_magics[square] = magic;
        }
    }
}

// Get rook masks
static Bitboard get_rook_mask(int square) {
    const Bitboard files = UINT64_C(0x0001010101010100);
    const Bitboard ranks = UINT64_C(0x7E);

    Bitboard attacks = (files << (square % 8)) | (ranks << (8 * (square / 8)));
    clear_bit(&attacks, square);

    return attacks;
}

// Get bishop masks
static Bitboard get_bishop_mask(int square) {
    Bitboard attacks = UINT64_C(0);
    int rank = square / 8, file = square % 8, r, f;

    for (r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++) {
        set_bit(&attacks, r * 8 + f);
    }
    for (r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--) {
        set_bit(&attacks, r * 8 + f);
    }
    for (r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++) {
        set_bit(&attacks, r * 8 + f);
    }
    for (r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--) {
        set_bit(&attacks, r * 8 + f);
    }

    return attacks;
}

// Get slider attacks given an occupancy bitboard to compute magics
static Bitboard get_slider_attack(int square, Bitboard occupancy, int piece) {
    Bitboard attacks = UINT64_C(0);
    int rook_direction[4] = {UP, DOWN, LEFT, RIGHT};
    int bishop_direction[4] = {UPRIGHT, UPLEFT, DOWNRIGHT, DOWNLEFT};

    for (int i = 0; i < 4; i++) {
        int direction = piece == ROOK ? rook_direction[i] : bishop_direction[i];
        int current = square;

        while (!get_bit(occupancy, current) && in_bounds(current, direction)) {
            set_bit(&attacks, current + direction);
            current += direction;
        }
    }

    return attacks;
}

// Get rook attacks using magic lookup tables and perfect hashing algorithm
static inline Bitboard get_rook_attacks(int square, Bitboard occupancy) {
    Magic m = rook_magics[square];
    return m.attacks[((occupancy & m.mask) * m.magic) >> m.shift];
}

// Get bishop attacks using magic lookup tables and perfect hashing algorithm
static inline Bitboard get_bishop_attacks(int square, Bitboard occupancy) {
    Magic m = bishop_magics[square];
    return m.attacks[((occupancy & m.mask) * m.magic) >> m.shift];
}

// Generate random 64 bit number with 1/8th bits set on average
static inline Bitboard sparse_random() {
    // Fastest seed out of 10 billion starting seeds
    static Bitboard seed = UINT64_C(0xAE793F42471A8799);
    Bitboard rand = ~UINT64_C(0);

    // Pseudo random number generator
    for (int i = 0; i < 3; i++) {
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;
        rand &= seed * UINT64_C(0x2545F4914F6CDD1D);
    }

    return rand;
}
